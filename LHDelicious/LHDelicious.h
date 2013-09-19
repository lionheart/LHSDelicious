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
    LHDeliciousErrorBookmarkNotFound,
    LHDeliciousErrorEmptyResponse
};

typedef void(^LHDeliciousGenericBlock)(id);
typedef void(^LHDeliciousStringBlock)(NSString *);
typedef void(^LHDeliciousDateBlock)(NSDate *);
typedef void(^LHDeliciousEmptyBlock)();
typedef void(^LHDeliciousDictionaryBlock)();
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

#pragma mark Bookmarks
- (void)bookmarksWithSuccess:(LHDeliciousSuccessBlock)success failure:(LHDeliciousErrorBlock)failure;
- (void)bookmarksWithTags:(NSString *)tags
               offset:(NSInteger)offset
                count:(NSInteger)count
             fromDate:(NSDate *)fromDate
               toDate:(NSDate *)toDate
          includeMeta:(BOOL)includeMeta
              success:(LHDeliciousSuccessBlock)success
              failure:(LHDeliciousErrorBlock)failure;

- (void)addBookmark:(NSDictionary *)bookmark
            success:(void (^)())success
            failure:(void (^)(NSError *))failure;

- (void)addBookmarkWithURL:(NSString *)url
                     title:(NSString *)title
               description:(NSString *)description
                      tags:(NSString *)tags
                    shared:(BOOL)shared
                    unread:(BOOL)unread
                   success:(LHDeliciousEmptyBlock)success
                   failure:(LHDeliciousErrorBlock)failure;

- (void)bookmarkWithURL:(NSString *)url success:(LHDeliciousDictionaryBlock)success failure:(LHDeliciousErrorBlock)failure;
- (void)deleteBookmarkWithURL:(NSString *)url success:(LHDeliciousEmptyBlock)success failure:(LHDeliciousErrorBlock)failure;

#pragma mark Tags
- (void)tagsWithSuccess:(LHDeliciousDictionaryBlock)success;
- (void)deleteTag:(NSString *)tag success:(LHDeliciousEmptyBlock)success;
- (void)renameTagFrom:(NSString *)oldTag to:(NSString *)newTag success:(LHDeliciousEmptyBlock)success;

#pragma mark Tag bundles
- (void)tagBundlesWithSuccess:(LHDeliciousDictionaryBlock)success;
- (void)updateTagBundle:(NSString *)bundle withTags:(NSString *)tags success:(LHDeliciousEmptyBlock)success;
- (void)deleteTagBundle:(NSString *)bundle success:(LHDeliciousEmptyBlock)success;

@end
