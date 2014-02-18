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
    if (self = [super init]) {
        // Initialize request callback blocks
        self.requestStartedCallback = ^{};
        self.requestCompletedCallback = ^{};
        
        // Setup date formatter
        self.dateFormatter = [[NSDateFormatter alloc] init];
        [self.dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss'Z'"];
        [self.dateFormatter setTimeZone:[NSTimeZone timeZoneWithAbbreviation:@"UTC"]];
        
        // HTTP client
        self.httpClient = [AFHTTPClient clientWithBaseURL:[LHSDelicious endpointURL]];
        //[self.httpClient registerHTTPOperationClass:[AFXMLRequestOperation class]];
        
        // XML parser settings
        [[XMLDictionaryParser sharedInstance] setAlwaysUseArrays:YES];
        [[XMLDictionaryParser sharedInstance] setAttributesMode:XMLDictionaryAttributesModeUnprefixed];
    }
    return self;
}

#pragma mark Request wrappers
- (void)requestPath:(NSString *)path parameters:(NSDictionary *)parameters success:(LHSDeliciousGenericBlock)success failure:(LHSDeliciousErrorBlock)failure {
    
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
    
    // Construct the query string and URL
    NSString *queryString = [queryComponents componentsJoinedByString:@"&"];
    NSString *urlString = [NSString stringWithFormat:@"%@%@?%@", DeliciousEndpoint, path, queryString];
    NSURL *url = [NSURL URLWithString:urlString];
    
    // Create the request
    NSMutableURLRequest *request = [self.httpClient requestWithMethod:@"GET" path:[url absoluteString] parameters:nil];
    AFHTTPRequestOperation *operation = [[AFHTTPRequestOperation alloc] initWithRequest:request];
    
    // Setup HTTP authentication
    [operation setWillSendRequestForAuthenticationChallengeBlock:^(NSURLConnection *connection, NSURLAuthenticationChallenge *challenge) {
        NSURLCredential *credential = [NSURLCredential credentialWithUser:self.username password:self.password persistence:NSURLCredentialPersistenceForSession];
        [[challenge sender] useCredential:credential forAuthenticationChallenge:challenge];
    }];
    
    // Set the completion blocks
    [operation setCompletionBlockWithSuccess:^(AFHTTPRequestOperation *operation, id responseObject) {
        success(responseObject);
        self.requestCompletedCallback();
    } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
        // TODO: Parse other errors here
        failure([NSError errorWithDomain:LHSDeliciousErrorDomain code:LHSDeliciousErrorEmptyResponse userInfo:nil]);
        self.requestCompletedCallback();
    }];
    
    // Start the request
    [operation start];
    
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
                         success:(LHSDeliciousStringBlock)success
                         failure:(LHSDeliciousErrorBlock)failure {
    // Update our global username/password
    self.username = username;
    self.password = password;
    
    // Delicious has no specific username check, so we check for a 200 OK on the last update time
    [self lastUpdateWithSuccess:^(NSDate *lastUpdate) {
        success(self.username);
    } failure:^(NSError *error) {
        failure([NSError errorWithDomain:LHSDeliciousErrorDomain code:LHSDeliciousErrorInvalidCredentials userInfo:nil]);
    }];
}

- (void)authenticateWithUsername:(NSString *)user
                        password:(NSString *)pass
                         success:(LHSDeliciousStringBlock)success
                         failure:(LHSDeliciousErrorBlock)failure {
    [self authenticateWithUsername:user password:pass timeout:30 success:success failure:failure];
}

- (void)resetAuthentication {
    self.username = nil;
    self.password = nil;
}

#pragma mark - API methods

#pragma mark Utility

// Get the NSDate of the last post update
- (void)lastUpdateWithSuccess:(LHSDeliciousDateBlock)success failure:(LHSDeliciousErrorBlock)failure {
    [self requestPath:@"posts/update" success:^(id response) {
        success([NSDate date]);
    } failure:^(NSError *error) {
        failure(error);
    }];
}

#pragma mark Bookmarks

// Get all posts
- (void)bookmarksWithSuccess:(LHSDeliciousSuccessBlock)success failure:(LHSDeliciousErrorBlock)failure {
    [self bookmarksWithTag:nil offset:-1 count:-1 fromDate:nil toDate:nil includeMeta:YES success:success failure:failure];
}

// Get posts with optional tag filter, offset, result count, date range, and meta data
- (void)bookmarksWithTag:(NSString *)tag
                  offset:(NSInteger)offset
                   count:(NSInteger)count
                fromDate:(NSDate *)fromDate
                  toDate:(NSDate *)toDate
             includeMeta:(BOOL)includeMeta
                 success:(LHSDeliciousSuccessBlock)success
                 failure:(LHSDeliciousErrorBlock)failure {
    
    // Build our paramters
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
        success([xml arrayValueForKeyPath:@"post"], parameters);
    } failure:failure];
}

// Add a new bookmark
- (void)addBookmark:(NSDictionary *)bookmark
            success:(void (^)())success
            failure:(void (^)(NSError *))failure {
    [self requestPath:@"posts/add" parameters:bookmark success:^(id response) {
        success();
    } failure:failure];
}

// Add a new bookmark with options
- (void)addBookmarkWithURL:(NSString *)url
                     title:(NSString *)title
               description:(NSString *)description
                      tags:(NSString *)tags
                    shared:(BOOL)shared
                   success:(LHSDeliciousEmptyBlock)success
                   failure:(LHSDeliciousErrorBlock)failure {
    NSDictionary *bookmark = @{
                               @"url": url,
                               @"description": title,
                               @"extended": description,
                               @"shared": shared ? @"yes" : @"no",
                               @"tags": tags,
                           };
    [self addBookmark:bookmark success:success failure:failure];
}

// Get a single bookmark
- (void)bookmarkWithURL:(NSString *)url success:(LHSDeliciousDictionaryBlock)success failure:(LHSDeliciousErrorBlock)failure {
    [self requestPath:@"posts/get" parameters:@{@"url": url} success:^(id response) {
        if ([response[@"posts"] count] == 0) {
            failure([NSError errorWithDomain:LHSDeliciousErrorDomain code:LHSDeliciousErrorBookmarkNotFound userInfo:nil]);
        }
        else {
            NSDictionary *xml = [NSDictionary dictionaryWithXMLData:(NSData *)response];
            success([xml arrayValueForKeyPath:@"post"][0]);
        }
    } failure:failure];
}

// Delete bookmark
- (void)deleteBookmarkWithURL:(NSString *)url success:(LHSDeliciousEmptyBlock)success failure:(LHSDeliciousErrorBlock)failure {
    [self requestPath:@"posts/delete" parameters:@{@"url": url} success:^(id response) {
        success();
    } failure:failure];
}

#pragma mark Tags

// Get all tags
- (void)tagsWithSuccess:(LHSDeliciousDictionaryBlock)success {
    [self requestPath:@"tags/get" success:^(id response) {
        NSDictionary *xml = [NSDictionary dictionaryWithXMLData:(NSData *)response];
        NSMutableDictionary *tagDict = [[NSMutableDictionary alloc] init];
        for (NSDictionary *tag in [xml arrayValueForKeyPath:@"tag"]) {
            [tagDict setValue:[tag valueForKey:@"count"] forKey:[tag valueForKeyPath:@"tag"]];
        }
        success(tagDict);
    }];
}

// Delete a tag
- (void)deleteTag:(NSString *)tag success:(LHSDeliciousEmptyBlock)success {
    [self requestPath:@"tags/delete" parameters:@{@"tag": tag} success:^(id response) {
        success();
    } failure:^(NSError *error) {}];
}

// Rename a tag
- (void)renameTagFrom:(NSString *)oldTag to:(NSString *)newTag success:(LHSDeliciousEmptyBlock)success {
    [self requestPath:@"tags/rename" parameters:@{@"old": oldTag, @"new": newTag} success:^(id response) {
        success();
    } failure:^(NSError *error) {}];
}

#pragma mark Tag bundles

// Get all tag bundles
- (void)tagBundlesWithSuccess:(LHSDeliciousDictionaryBlock)success {
    [self requestPath:@"tags/bundles/all" success:^(id response) {
        NSDictionary *xml = [NSDictionary dictionaryWithXMLData:(NSData *)response];
        NSMutableDictionary *tagDict = [[NSMutableDictionary alloc] init];
        for (NSDictionary *tag in [xml arrayValueForKeyPath:@"tag"]) {
            [tagDict setValue:[tag valueForKey:@"tags"] forKey:[tag valueForKeyPath:@"name"]];
        }
        success(tagDict);
    }];
}

// Update a tag bundle
- (void)updateTagBundle:(NSString *)bundle withTags:(NSString *)tags success:(LHSDeliciousEmptyBlock)success {
    [self requestPath:@"tags/bundles/set" parameters:@{@"bundle": bundle, @"tags": tags} success:^(id response) {
        success();
    } failure:^(NSError *error) {}];
}

// Delete a tag bundle
- (void)deleteTagBundle:(NSString *)bundle success:(LHSDeliciousEmptyBlock)success {
    [self requestPath:@"tags/bundles/delete" parameters:@{@"bundle": bundle} success:^(id response) {
        success();
    } failure:^(NSError *error) {}];
}

@end
