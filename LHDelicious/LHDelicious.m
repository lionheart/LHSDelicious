//
//  LHDelicious.m
//  LHDelicious
//
//  Created by Andy Muldowney on 9/18/13.
//  Copyright (c) 2013 Lionheart Software. All rights reserved.
//

#import "LHDelicious.h"
#import "NSString+URLEncoding.h"
#import <XMLDictionary/XMLDictionary.h>

@implementation LHDelicious

@synthesize username, password;
@synthesize dateFormatter;
@synthesize throttleTimer;
@synthesize requestStartedCallback, requestCompletedCallback;

+ (LHDelicious *)sharedInstance {
    static LHDelicious *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[LHDelicious alloc] init];
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
        
        self.httpClient = [AFHTTPClient clientWithBaseURL:[LHDelicious endpointURL]];
        //[self.httpClient registerHTTPOperationClass:[AFXMLRequestOperation class]];
    }
    return self;
}

#pragma mark Request wrappers
- (void)requestPath:(NSString *)path parameters:(NSDictionary *)parameters success:(LHDeliciousGenericBlock)success failure:(LHDeliciousErrorBlock)failure {
    
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
    } failure:^(AFHTTPRequestOperation *operation, NSError *error) {
        // TODO: Parse other errors here
        failure([NSError errorWithDomain:LHDeliciousErrorDomain code:LHDeliciousErrorEmptyResponse userInfo:nil]);
    }];
    
    // Start the request
    [operation start];
    
}

- (void)requestPath:(NSString *)path success:(LHDeliciousGenericBlock)success failure:(LHDeliciousErrorBlock)failure {
    [self requestPath:path parameters:nil success:success failure:failure];
}

- (void)requestPath:(NSString *)path success:(LHDeliciousGenericBlock)success {
    [self requestPath:path parameters:nil success:success failure:^(NSError *error){}];
}

#pragma mark Authentication
- (void)authenticateWithUsername:(NSString *)user password:(NSString *)pass timeout:(NSTimeInterval)timeout success:(LHDeliciousStringBlock)success failure:(LHDeliciousErrorBlock)failure {
    // Update our global username/password
    self.username = user;
    self.password = pass;
    
    // Delicious has no specific username check, so we check for a 200 OK on the last update time
    [self lastUpdateWithSuccess:^(NSDate *lastUpdate) {
        success(self.username);
    } failure:^(NSError *error) {
        failure([NSError errorWithDomain:LHDeliciousErrorDomain code:LHDeliciousErrorInvalidCredentials userInfo:nil]);
    }];
}

#pragma mark - API methods

#pragma mark Utility

// Get the NSDate of the last post update
- (void)lastUpdateWithSuccess:(LHDeliciousDateBlock)success failure:(LHDeliciousErrorBlock)failure {
    [self requestPath:@"posts/update" success:^(id response) {
        success([NSDate date]);
    } failure:^(NSError *error) {
        failure(error);
    }];
}

#pragma mark Posts

// Get all posts
- (void)postsWithSuccess:(LHDeliciousSuccessBlock)success failure:(LHDeliciousErrorBlock)failure {
    [self postsWithTags:nil offset:-1 count:-1 fromDate:nil toDate:nil includeMeta:YES success:success failure:failure];
}

// Get posts with optional tag filter, offset, result count, date range, and meta data
- (void)postsWithTags:(NSString *)tags
               offset:(NSInteger)offset
                count:(NSInteger)count
             fromDate:(NSDate *)fromDate
               toDate:(NSDate *)toDate
          includeMeta:(BOOL)includeMeta
              success:(LHDeliciousSuccessBlock)success
              failure:(LHDeliciousErrorBlock)failure {
    
    // Build our paramters
    NSMutableDictionary *parameters = [NSMutableDictionary dictionary];
    if (tags) parameters[@"tags"] = tags;
    if (offset != -1) parameters[@"start"] = [NSString stringWithFormat:@"%d", offset];
    if (count != -1 ) parameters[@"results"] = [NSString stringWithFormat:@"%d", count];
    if (fromDate) parameters[@"fromdt"] = [self.dateFormatter stringFromDate:fromDate];
    if (toDate) parameters[@"todt"] = [self.dateFormatter stringFromDate:fromDate];
    if (includeMeta) parameters[@"meta"] = @"yes";
    
    // Start the request
    [self requestPath:@"posts/all" success:^(id response) {
        // Parse the XML data
        [[XMLDictionaryParser sharedInstance] setAlwaysUseArrays:YES];
        NSDictionary *xml = [NSDictionary dictionaryWithXMLData:(NSData *)response];
        success([xml arrayValueForKeyPath:@"post"], nil);
    } failure:failure];
}

// Add a new bookmark

#pragma mark Tags


#pragma mark Tag bundles

@end
