//
//  LHSDelicious.m
//  LHSDelicious
//
//  Created by Andy Muldowney on 9/18/13.
//  Copyright (c) 2013 Lionheart Software. All rights reserved.
//

#import "LHSDelicious.h"
#import "NSString+URLEncoding.h"
#import <XMLDictionary/XMLDictionary.h>


@implementation LHSDelicious

@synthesize username = _username;
@synthesize password = _password;

+ (LHSDelicious *)sharedInstance {
    static LHSDelicious *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[LHSDelicious alloc] init];
    });
    return sharedInstance;
}

+ (NSURL *)endpointURL {
    return [NSURL URLWithString:DeliciousEndpoint];
}

- (id)init {
    if (self = [super initWithBaseURL:[LHSDelicious endpointURL]]) {
        // Initialize request callback blocks
        self.requestStartedCallback = ^{};
        self.requestCompletedCallback = ^{};
        
        self.responseSerializer = [AFHTTPResponseSerializer serializer];
        self.requestSerializer = [AFHTTPRequestSerializer serializer];
        
        // Setup date formatter
        self.dateFormatter = [[NSDateFormatter alloc] init];
        [self.dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss'Z'"];
        [self.dateFormatter setTimeZone:[NSTimeZone timeZoneWithAbbreviation:@"UTC"]];

        // XML parser settings
        [[XMLDictionaryParser sharedInstance] setAlwaysUseArrays:YES];
        [[XMLDictionaryParser sharedInstance] setAttributesMode:XMLDictionaryAttributesModeUnprefixed];
    }
    return self;
}

#pragma mark Request wrappers
- (void)requestPath:(NSString *)path
         parameters:(NSDictionary *)parameters
            success:(LHSDeliciousGenericBlock)success
            failure:(LHSDeliciousErrorBlock)failure {
    
    // Initiate the request started callback
    self.requestStartedCallback();
    
    // Build the query components
    NSMutableArray *queryComponents = [NSMutableArray array];
    [parameters enumerateKeysAndObjectsUsingBlock:^(NSString *key, NSString *value, BOOL *stop) {
        if (![value isEqualToString:@""]) {
            [queryComponents addObject:[NSString stringWithFormat:@"%@=%@", [key urlEncode], [value urlEncode]]];
        }
    }];
    
    if (!failure) failure = ^(NSError *error) {};

    self.credential = [NSURLCredential credentialWithUser:self.username
                                                 password:self.password
                                              persistence:NSURLCredentialPersistenceForSession];
    // Create the request
    [self GET:path
   parameters:parameters
      success:^(AFHTTPRequestOperation *task, id response) {
          success(response);
          self.requestCompletedCallback();
      }
      failure:^(AFHTTPRequestOperation *task, NSError *error) {
          // TODO: Parse other errors here
          failure([NSError errorWithDomain:LHSDeliciousErrorDomain code:LHSDeliciousErrorEmptyResponse userInfo:nil]);
          self.requestCompletedCallback();
      }];
}

- (void)requestPath:(NSString *)path success:(LHSDeliciousGenericBlock)success failure:(LHSDeliciousErrorBlock)failure {
    [self requestPath:path parameters:nil success:success failure:failure];
}

- (void)requestPath:(NSString *)path success:(LHSDeliciousGenericBlock)success {
    [self requestPath:path parameters:nil success:success failure:^(NSError *error){}];
}

#pragma mark Authentication

- (void)authenticateWithUsername:(NSString *)username
                        password:(NSString *)password
                         timeout:(NSTimeInterval)timeout
                      completion:(LHSDeliciousErrorBlock)completion {
    // Update our global username/password
    self.username = username;
    self.password = password;
    
    // Delicious has no specific username check, so we check for a 200 OK on the last update time
    [self lastUpdateWithSuccess:^(NSDate *lastUpdate) {
        completion(nil);
    } failure:^(NSError *error) {
        completion([NSError errorWithDomain:LHSDeliciousErrorDomain code:LHSDeliciousErrorInvalidCredentials userInfo:nil]);
    }];
}

- (void)authenticateWithUsername:(NSString *)username
                        password:(NSString *)password
                      completion:(LHSDeliciousErrorBlock)completion {
    
    [self authenticateWithUsername:user
                          password:pass
                           timeout:30
                        completion:completion];
}

- (void)resetAuthentication {
    self.username = nil;
    self.password = nil;
}

#pragma mark - API methods

#pragma mark Utility

- (void)lastUpdateWithCompletion:(LHSDeliciousDateErrorBlock)completion {
    [self requestPath:@"posts/update" success:^(id response) {
        completion([NSDate date], nil);
    } failure:^(NSError *error) {
        completion(nil, error);
    }];
}

#pragma mark Bookmarks

- (void)bookmarksWithCompletion:(LHSDeliciousArrayDictionaryErrorBlock)completion {
    [self bookmarksWithTag:nil
                    offset:-1
                     count:-1
                  fromDate:nil
                    toDate:nil
               includeMeta:YES
                completion:completion];
}

- (void)bookmarksWithTag:(NSString *)tag
                  offset:(NSInteger)offset
                   count:(NSInteger)count
                fromDate:(NSDate *)fromDate
                  toDate:(NSDate *)toDate
             includeMeta:(BOOL)includeMeta
              completion:(LHSDeliciousArrayDictionaryErrorBlock)completion {
    NSMutableDictionary *parameters = [NSMutableDictionary dictionary];
    if (tag) parameters[@"tag"] = tag;
    if (offset != -1) parameters[@"start"] = [NSString stringWithFormat:@"%ld", (long)offset];
    if (count != -1 ) parameters[@"results"] = [NSString stringWithFormat:@"%ld", (long)count];
    if (fromDate) parameters[@"fromdt"] = [self.dateFormatter stringFromDate:fromDate];
    if (toDate) parameters[@"todt"] = [self.dateFormatter stringFromDate:fromDate];
    if (includeMeta) parameters[@"meta"] = @"yes";
    
    // Start the request
    [self requestPath:@"posts/all" parameters:parameters success:^(id response) {
        // Parse the XML data
        NSDictionary *xml = [NSDictionary dictionaryWithXMLData:(NSData *)response];
        completion([xml arrayValueForKeyPath:@"post"], parameters, nil);
    } failure:^(NSError *error) {
        completion(nil, nil, error);
    }];
}

// Add a new bookmark

- (void)addBookmark:(NSDictionary *)bookmark
         completion:(LHSDeliciousErrorBlock)completion {
    NSMutableDictionary *parameters = [bookmark mutableCopy];
    parameters[@"replace"] = @"yes";

    [self requestPath:@"posts/add" parameters:parameters success:^(id response) {
        completion(nil);
    } failure:completion];
}

- (void)addBookmarkWithURL:(NSString *)url
                     title:(NSString *)title
               description:(NSString *)description
                      tags:(NSString *)tags
                    shared:(BOOL)shared
                completion:(LHSDeliciousErrorBlock)completion {
    NSDictionary *bookmark = @{
                               @"url": url,
                               @"description": title,
                               @"extended": description,
                               @"shared": shared ? @"yes" : @"no",
                               @"tags": tags,
                               };
    [self addBookmark:bookmark completion:completion];
}

- (void)bookmarkWithURL:(NSString *)url completion:(LHSDeliciousDictionaryErrorBlock)completion {
    [self requestPath:@"posts/get" parameters:@{@"url": url} success:^(id response) {
        if ([response[@"posts"] count] == 0) {
            completion(nil, [NSError errorWithDomain:LHSDeliciousErrorDomain code:LHSDeliciousErrorBookmarkNotFound userInfo:nil]);
        }
        else {
            NSDictionary *xml = [NSDictionary dictionaryWithXMLData:(NSData *)response];
            completion([xml arrayValueForKeyPath:@"post"][0], nil);
        }
    } failure:^(NSError *error) {
        completion(nil, error);
    }];
}

- (void)deleteBookmarkWithURL:(NSString *)url
                   completion:(LHSDeliciousErrorBlock)completion {
    [self requestPath:@"posts/delete" parameters:@{@"url": url} success:^(id response) {
        completion(nil);
    } failure:completion];
}

#pragma mark Tags

- (void)tagsWithCompletion:(LHSDeliciousDictionaryBlock)completion {
    [self requestPath:@"tags/get" success:^(id response) {
        NSDictionary *xml = [NSDictionary dictionaryWithXMLData:(NSData *)response];
        NSMutableDictionary *tagDict = [[NSMutableDictionary alloc] init];
        for (NSDictionary *tag in [xml arrayValueForKeyPath:@"tag"]) {
            [tagDict setValue:[tag valueForKey:@"count"] forKey:[tag valueForKeyPath:@"tag"]];
        }

        completion(tagDict);
    }];
}

- (void)deleteTag:(NSString *)tag
       completion:(LHSDeliciousEmptyBlock)completion {
    [self requestPath:@"tags/delete" parameters:@{@"tag": tag} success:^(id response) {
        completion();
    } failure:^(NSError *error) {
        completion();
    }];
}

- (void)renameTagFrom:(NSString *)oldTag
                   to:(NSString *)newTag
           completion:(LHSDeliciousEmptyBlock)completion {
    [self requestPath:@"tags/rename" parameters:@{@"old": oldTag, @"new": newTag} success:^(id response) {
        completion();
    } failure:^(NSError *error) {
        completion();
    }];
}

#pragma mark Tag bundles

- (void)tagBundlesWithCompletion:(LHSDeliciousDictionaryBlock)completion {
    [self requestPath:@"tags/bundles/all" success:^(id response) {
        NSDictionary *xml = [NSDictionary dictionaryWithXMLData:(NSData *)response];
        NSMutableDictionary *tagDict = [[NSMutableDictionary alloc] init];
        for (NSDictionary *tag in [xml arrayValueForKeyPath:@"tag"]) {
            [tagDict setValue:[tag valueForKey:@"tags"] forKey:[tag valueForKeyPath:@"name"]];
        }
        completion(tagDict);
    }];
}

- (void)updateTagBundle:(NSString *)bundle
               withTags:(NSString *)tags
             completion:(LHSDeliciousEmptyBlock)completion {
    [self requestPath:@"tags/bundles/set" parameters:@{@"bundle": bundle, @"tags": tags} success:^(id response) {
        completion();
    } failure:^(NSError *error) {
        completion();
    }];
}

- (void)deleteTagBundle:(NSString *)bundle
             completion:(LHSDeliciousEmptyBlock)completion {
    [self requestPath:@"tags/bundles/delete" parameters:@{@"bundle": bundle} success:^(id response) {
        completion();
    } failure:^(NSError *error) {
        completion();
    }];
}

@end
