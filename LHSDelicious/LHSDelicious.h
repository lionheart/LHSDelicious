//
//  LHSDelicious.h
//  LHSDelicious
//
//  Created by Andy Muldowney on 9/18/13.
//  Copyright (c) 2013 Lionheart Software. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AFNetworking/AFHTTPRequestOperationManager.h>

static NSString *DeliciousEndpoint __unused = @"https://api.delicious.com/v1/";
static NSString *LHSDeliciousErrorDomain __unused = @"LHDelicousErrorDomain";

enum DELICIOUS_ERROR_CODES {
    LHSDeliciousErrorTimeout,
    LHSDeliciousErrorThrottled,
    LHSDeliciousErrorInvalidCredentials,
    LHSDeliciousErrorBookmarkNotFound,
    LHSDeliciousErrorEmptyResponse
};

typedef void(^LHSDeliciousGenericBlock)(id);
typedef void(^LHSDeliciousStringBlock)(NSString *);
typedef void(^LHSDeliciousDateBlock)(NSDate *);
typedef void(^LHSDeliciousEmptyBlock)();
typedef void(^LHSDeliciousDictionaryBlock)();
typedef void(^LHSDeliciousSuccessBlock)(NSArray *, NSDictionary *);
typedef void(^LHSDeliciousErrorBlock)(NSError *);

@interface LHSDelicious : AFHTTPRequestOperationManager <NSURLConnectionDataDelegate, NSURLConnectionDelegate>

@property (nonatomic, retain) NSString *username;
@property (nonatomic, retain) NSString *password;
@property (nonatomic, retain) NSDateFormatter *dateFormatter;
@property (nonatomic, retain) NSTimer *throttleTimer;

@property (nonatomic, copy) void (^requestStartedCallback)();
@property (nonatomic, copy) void (^requestCompletedCallback)();
@property (nonatomic, copy) void (^loginTimeoutCallback)();

+ (NSString *)urlEncode;
+ (NSURL *)endpointURL;

/**
 Creates and returns an `LHSDelicious` object.
 */
+ (LHSDelicious *)sharedInstance;

- (void)requestPath:(NSString *)path parameters:(NSDictionary *)parameters success:(LHSDeliciousGenericBlock)success failure:(LHSDeliciousErrorBlock)failure;
- (void)requestPath:(NSString *)path success:(LHSDeliciousGenericBlock)success failure:(LHSDeliciousErrorBlock)failure;
- (void)requestPath:(NSString *)path success:(LHSDeliciousGenericBlock)success;

- (void)authenticateWithUsername:(NSString *)username
                        password:(NSString *)password
                         timeout:(NSTimeInterval)timeout
                         success:(LHSDeliciousStringBlock)success
                         failure:(LHSDeliciousErrorBlock)failure;

- (void)authenticateWithUsername:(NSString *)username
                        password:(NSString *)password
                         success:(LHSDeliciousStringBlock)success
                         failure:(LHSDeliciousErrorBlock)failure;

- (void)resetAuthentication;

#pragma mark - API functions

///--------------------------------
/// @name Primary API functionality
///--------------------------------

#pragma mark Utility

/**
 Retrieve the time bookmarks were last updated on the server.
 
 @param success A block object to be executed when the request finishes successfully. This block has no return value and takes one argument: a date representing the time that bookmarks were last updated on the server.
 @param failure A block object to be executed when the task finishes unsuccessfully, or that finishes successfully, but encountered an error while parsing the response data. This block has no return value and takes one argument: an error describing the network or parsing error that occurred.
 */
- (void)lastUpdateWithSuccess:(LHSDeliciousDateBlock)success failure:(LHSDeliciousErrorBlock)failure;

#pragma mark Bookmarks
- (void)bookmarksWithSuccess:(LHSDeliciousSuccessBlock)success failure:(LHSDeliciousErrorBlock)failure;
- (void)bookmarksWithTag:(NSString *)tag
                  offset:(NSInteger)offset
                   count:(NSInteger)count
                fromDate:(NSDate *)fromDate
                  toDate:(NSDate *)toDate
             includeMeta:(BOOL)includeMeta
                 success:(LHSDeliciousSuccessBlock)success
                 failure:(LHSDeliciousErrorBlock)failure;

- (void)addBookmark:(NSDictionary *)bookmark
            success:(void (^)())success
            failure:(void (^)(NSError *))failure;

- (void)addBookmarkWithURL:(NSString *)url
                     title:(NSString *)title
               description:(NSString *)description
                      tags:(NSString *)tags
                    shared:(BOOL)shared
                   success:(LHSDeliciousEmptyBlock)success
                   failure:(LHSDeliciousErrorBlock)failure;

- (void)bookmarkWithURL:(NSString *)url success:(LHSDeliciousDictionaryBlock)success failure:(LHSDeliciousErrorBlock)failure;
- (void)deleteBookmarkWithURL:(NSString *)url success:(LHSDeliciousEmptyBlock)success failure:(LHSDeliciousErrorBlock)failure;

#pragma mark Tags
- (void)tagsWithSuccess:(LHSDeliciousDictionaryBlock)success;
- (void)deleteTag:(NSString *)tag success:(LHSDeliciousEmptyBlock)success;
- (void)renameTagFrom:(NSString *)oldTag to:(NSString *)newTag success:(LHSDeliciousEmptyBlock)success;

#pragma mark Tag bundles
- (void)tagBundlesWithSuccess:(LHSDeliciousDictionaryBlock)success;
- (void)updateTagBundle:(NSString *)bundle withTags:(NSString *)tags success:(LHSDeliciousEmptyBlock)success;
- (void)deleteTagBundle:(NSString *)bundle success:(LHSDeliciousEmptyBlock)success;

@end
