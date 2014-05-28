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

/*
 * Creates and returns an `LHSDelicious` object.
 */
+ (LHSDelicious *)sharedInstance;

- (void)requestPath:(NSString *)path parameters:(NSDictionary *)parameters success:(LHSDeliciousGenericBlock)success failure:(LHSDeliciousErrorBlock)failure;
- (void)requestPath:(NSString *)path success:(LHSDeliciousGenericBlock)success failure:(LHSDeliciousErrorBlock)failure;
- (void)requestPath:(NSString *)path success:(LHSDeliciousGenericBlock)success;

/**
 *  Authenticate a user by providing a username, password, and timeout.
 *
 *  @param username Delicious username
 *  @param password Delicious password
 *  @param timeout  Timeout in seconds
 *  @param success  The block to be executed on the completion of a successful authentication request. The block has no return value and takes one argument: the username originally passed into the method.
 *  @param failure  The block to be executed on the completion of a failed authentication request. The block has no return value and takes one argument: the error that occurred during the request.
 */
- (void)authenticateWithUsername:(NSString *)username
                        password:(NSString *)password
                         timeout:(NSTimeInterval)timeout
                         success:(LHSDeliciousStringBlock)success
                         failure:(LHSDeliciousErrorBlock)failure;

/**
 *  Authenticate a user by providing a username and password.
 *
 *  @param username Delicious username
 *  @param password Delicious password
 *  @param timeout  Timeout in seconds
 *  @param success  The block to be executed on the completion of a successful authentication request. The block has no return value and takes one argument: the username originally passed into the method.
 *  @param failure  The block to be executed on the completion of a failed authentication request. The block has no return value and takes one argument: the error that occurred during the request.
 */
- (void)authenticateWithUsername:(NSString *)username
                        password:(NSString *)password
                         success:(LHSDeliciousStringBlock)success
                         failure:(LHSDeliciousErrorBlock)failure;

- (void)resetAuthentication;

#pragma mark - API functions

/** @name General */

#pragma mark Utility

/**
 *  Retrieve the time bookmarks were last updated on the server.
 *
 *  @param success A block object to be executed when the request finishes successfully. This block has no return value and takes one argument: a date representing the time that bookmarks were last updated on the server.
 *  @param failure A block object to be executed when the task finishes unsuccessfully, or that finishes successfully, but encountered an error while parsing the response data. This block has no return value and takes one argument: an error describing the network or parsing error that occurred.
 */
- (void)lastUpdateWithSuccess:(LHSDeliciousDateBlock)success failure:(LHSDeliciousErrorBlock)failure;

#pragma mark Bookmarks

/** @name Bookmarks */

/**
 *  Retrieve up to 100,000 bookmarks with no filters. Calls bookmarksWithTag:offset:count:fromDate:toDate:includeMeta:success:failure:.
 *
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes one argument: an array containing dictionary objects for each bookmark retrieved.
 *  @param failure A block object to be executed when the request finishes unsuccessfully. This block has no return value and takes one argument: the error that occurred during the request.
 */
- (void)bookmarksWithSuccess:(LHSDeliciousSuccessBlock)success failure:(LHSDeliciousErrorBlock)failure;

/**
 *  Retrieve bookmarks using Delicious filters.
 *
 *  @param tag         The tag to filter by.
 *  @param offset      Offset results by this amount.
 *  @param count       The number of bookmarks to retrieve.
 *  @param fromDate    A date object describing the earliest date from which to retrieve bookmarks.
 *  @param toDate      A date object describing the latest date from which to retrieve bookmarks.
 *  @param includeMeta Include a change detection value in the bookmark response.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes one argument: an array containing dictionary objects for each bookmark retrieved.
 *  @param failure A block object to be executed when the request finishes unsuccessfully. This block has no return value and takes one argument: the error that occurred during the request.
 */

- (void)bookmarksWithTag:(NSString *)tag
                  offset:(NSInteger)offset
                   count:(NSInteger)count
                fromDate:(NSDate *)fromDate
                  toDate:(NSDate *)toDate
             includeMeta:(BOOL)includeMeta
                 success:(LHSDeliciousSuccessBlock)success
                 failure:(LHSDeliciousErrorBlock)failure;

/**
 *  Add a bookmark.
 *
 *  @param bookmark A dictionary object containing keys for 'url', 'title', 'description', 'shared', and 'tags'.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 *  @param failure A block object to be executed when the request finishes unsuccessfully. This block has no return value and takes one argument: the error that occurred during the request.
 */
- (void)addBookmark:(NSDictionary *)bookmark
            success:(void (^)())success
            failure:(void (^)(NSError *))failure;

/**
 *  Add a bookmark.
 *
 *  @param url         The URL for the bookmark
 *  @param title       Required. The bookmark's title. Corresponds to the "description" field in the Delicious API.
 *  @param description Optional. The bookmark's description. Corresponds to the "extended" field in the Delicious API.
 *  @param tags        Optional. Comma-separated tags for the bookmark.
 *  @param shared      Optional. Defaults to YES. If no, make the bookmark private.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 *  @param failure A block object to be executed when the request finishes unsuccessfully. This block has no return value and takes one argument: the error that occurred during the request.
 */
- (void)addBookmarkWithURL:(NSString *)url
                     title:(NSString *)title
               description:(NSString *)description
                      tags:(NSString *)tags
                    shared:(BOOL)shared
                   success:(LHSDeliciousEmptyBlock)success
                   failure:(LHSDeliciousErrorBlock)failure;

/**
 *  Retrieve a single bookmark.
 *
 *  @param url     The URL for the bookmark to retrieve.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes one argument: a dictionary describing the bookmark object that was retrieved.
 *  @param failure A block object to be executed when the request finishes unsuccessfully. This block has no return value and takes one argument: the error that occurred during the request.
 */
- (void)bookmarkWithURL:(NSString *)url success:(LHSDeliciousDictionaryBlock)success failure:(LHSDeliciousErrorBlock)failure;

/**
 *  Delete a bookmark.
 *
 *  @param url     The URL for the bookmark to delete.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 *  @param failure A block object to be executed when the request finishes unsuccessfully. This block has no return value and takes one argument: the error that occurred during the request.
 */
- (void)deleteBookmarkWithURL:(NSString *)url success:(LHSDeliciousEmptyBlock)success failure:(LHSDeliciousErrorBlock)failure;

#pragma mark Tags

/** @name Tags */

/**
 *  Retrieve all tags.
 *
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes one argument: a dictionary with tag names as keys and frequencies as values.
 */
- (void)tagsWithSuccess:(LHSDeliciousDictionaryBlock)success;

/**
 *  Delete a tag.
 *
 *  @param tag     The name of the tag to delete.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 */
- (void)deleteTag:(NSString *)tag success:(LHSDeliciousEmptyBlock)success;

/**
 *  Rename a tag.
 *
 *  @param oldTag  The current name of the tag.
 *  @param newTag  The updated name of the tag.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 */
- (void)renameTagFrom:(NSString *)oldTag to:(NSString *)newTag success:(LHSDeliciousEmptyBlock)success;

#pragma mark Tag bundles

/** @name Tag Bundles */

/**
 *  Retrieve all tag bundles.
 *
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes one argument: a dictionary with the names of the tag bundles as keys and consituent tags as values.
 */
- (void)tagBundlesWithSuccess:(LHSDeliciousDictionaryBlock)success;

/**
 *  Update the tags included in a tag bundle.
 *
 *  @param bundle  The name of the tag bundle to update.
 *  @param tags    The new list of tags to include in the tag bundle.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 */
- (void)updateTagBundle:(NSString *)bundle withTags:(NSString *)tags success:(LHSDeliciousEmptyBlock)success;

/**
 *  Delete a tag bundle.
 *
 *  @param bundle  The name of the tag bundle to delete.
 *  @param success A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 */
- (void)deleteTagBundle:(NSString *)bundle success:(LHSDeliciousEmptyBlock)success;

@end
