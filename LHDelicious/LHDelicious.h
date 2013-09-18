//
//  LHDelicious.h
//  LHDelicious
//
//  Created by Andy Muldowney on 9/18/13.
//  Copyright (c) 2013 Lionheart Software. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AFNetworking/AFNetworking.h>

static NSString *DeliciousEndpoint __unused = @"https://api.delicious.com/v1/";
static NSString *LHDeliciousErrorDomain __unused = @"LHDelicousErrorDomain";

enum DELICIOUS_ERROR_CODES {
    LHDeliciousErrorTimeout,
    LHDeliciousErrorThrottled,
    LHDeliciousErrorInvalidCredentials,
    LHDeliciousErrorEmptyResponse
};

typedef void(^LHDeliciousGenericBlock)(id);
typedef void(^LHDeliciousStringBlock)(NSString *);
typedef void(^LHDeliciousDateBlock)(NSDate *);
typedef void(^LHDeliciousSuccessBlock)(NSArray *, NSDictionary *);
typedef void(^LHDeliciousErrorBlock)(NSError *);

@interface LHDelicious : NSObject <NSURLConnectionDataDelegate, NSURLConnectionDelegate>

@property (nonatomic, retain) NSString *username;
@property (nonatomic, retain) NSString *password;
@property (nonatomic, retain) NSDateFormatter *dateFormatter;
@property (nonatomic, retain) NSTimer *throttleTimer;
@property (nonatomic, retain) AFHTTPClient *httpClient;

@property (nonatomic, copy) void (^requestStartedCallback)();
@property (nonatomic, copy) void (^requestCompletedCallback)();
@property (nonatomic, copy) void (^loginTimeoutCallback)();

+ (NSString *)urlEncode;
+ (LHDelicious *)sharedInstance;
+ (NSURL *)endpointURL;

- (void)requestPath:(NSString *)path parameters:(NSDictionary *)parameters success:(LHDeliciousGenericBlock)success failure:(LHDeliciousErrorBlock)failure;
- (void)requestPath:(NSString *)path success:(LHDeliciousGenericBlock)success failure:(LHDeliciousErrorBlock)failure;
- (void)requestPath:(NSString *)path success:(LHDeliciousGenericBlock)success;

- (void)authenticateWithUsername:(NSString *)user
                        password:(NSString *)pass
                         timeout:(NSTimeInterval)timeout
                         success:(LHDeliciousStringBlock)success
                         failure:(LHDeliciousErrorBlock)failure;

- (void)authenticateWithUsername:(NSString *)user
                        password:(NSString *)pass
                         success:(LHDeliciousStringBlock)success
                         failure:(LHDeliciousErrorBlock)failure;

#pragma mark - API functions

#pragma mark Utility
- (void)lastUpdateWithSuccess:(LHDeliciousDateBlock)success failure:(LHDeliciousErrorBlock)failure;

#pragma mark Posts
- (void)postsWithSuccess:(LHDeliciousSuccessBlock)success failure:(LHDeliciousErrorBlock)failure;
- (void)postsWithTags:(NSString *)tags
               offset:(NSInteger)offset
                count:(NSInteger)count
             fromDate:(NSDate *)fromDate
               toDate:(NSDate *)toDate
          includeMeta:(BOOL)includeMeta
              success:(LHDeliciousSuccessBlock)success
              failure:(LHDeliciousErrorBlock)failure;

@end
