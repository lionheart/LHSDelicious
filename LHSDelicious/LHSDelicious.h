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
typedef void(^LHSDeliciousErrorBlock)(NSError *error);

typedef void(^LHSDeliciousArrayDictionaryErrorBlock)(NSArray *bookmarks, NSDictionary * NSError *error);
typedef void(^LHSDeliciousDictionaryErrorBlock)(NSDictionary *bookmark, NSError *error);
typedef void(^LHSDeliciousDateErrorBlock)(NSDate *date, NSError *error);

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
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes one argument: the error (if any) that occurred during the request.
 */
- (void)authenticateWithUsername:(NSString *)username
                        password:(NSString *)password
                         timeout:(NSTimeInterval)timeout
                      completion:(LHSDeliciousErrorBlock)completion;

/**
 *  Authenticate a user by providing a username and password.
 *
 *  @param username Delicious username
 *  @param password Delicious password
 *  @param timeout  Timeout in seconds
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes one argument: the error (if any) that occurred during the request.
 */
- (void)authenticateWithUsername:(NSString *)username
                        password:(NSString *)password
                      completion:(LHSDeliciousErrorBlock)completion

- (void)resetAuthentication;

#pragma mark - API functions

/** @name General */

#pragma mark Utility

/**
 *  Retrieve the time bookmarks were last updated on the server.
 *
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes two arguments: a date representing the time that bookmarks were last updated on the server and the error (if any) that occurred during the request.
 */
- (void)lastUpdateWithCompletion:(LHSDeliciousDateErrorBlock)completion;

#pragma mark Bookmarks

/** @name Bookmarks */

/**
 *  Retrieve up to 100,000 bookmarks with no filters. Calls bookmarksWithTag:offset:count:fromDate:toDate:includeMeta:success:failure:.
 *
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes two arguments: an array containing dictionary objects for each bookmark retrieved and the error (if any) that occurred during the request.
 */
- (void)bookmarksWithCompletion:(LHSDeliciousArrayDictionaryErrorBlock)completion;

/**
 *  Retrieve bookmarks using Delicious filters.
 *
 *  @param tag         The tag to filter by.
 *  @param offset      Offset results by this amount.
 *  @param count       The number of bookmarks to retrieve.
 *  @param fromDate    A date object describing the earliest date from which to retrieve bookmarks.
 *  @param toDate      A date object describing the latest date from which to retrieve bookmarks.
 *  @param includeMeta Include a change detection value in the bookmark response.
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes two arguments: an array containing dictionary objects for each bookmark retrieved and the error (if any) that occurred during the request.
 */

- (void)bookmarksWithTag:(NSString *)tag
                  offset:(NSInteger)offset
                   count:(NSInteger)count
                fromDate:(NSDate *)fromDate
                  toDate:(NSDate *)toDate
             includeMeta:(BOOL)includeMeta
              completion:(LHSDeliciousArrayDictionaryErrorBlock)completion;

/**
 *  Add a bookmark.
 *
 *  @param bookmark A dictionary object containing keys for 'url', 'title', 'description', 'shared', and 'tags'.
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes one argument: the error (if any) that occurred during the request.
 */
- (void)addBookmark:(NSDictionary *)bookmark
         completion:(LHSDeliciousErrorBlock)completion;

/**
 *  Add a bookmark.
 *
 *  @param url         The URL for the bookmark
 *  @param title       Required. The bookmark's title. Corresponds to the "description" field in the Delicious API.
 *  @param description Optional. The bookmark's description. Corresponds to the "extended" field in the Delicious API.
 *  @param tags        Optional. Comma-separated tags for the bookmark.
 *  @param shared      Optional. Defaults to YES. If no, make the bookmark private.
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes one argument: the error (if any) that occurred during the request.
 */
- (void)addBookmarkWithURL:(NSString *)url
                     title:(NSString *)title
               description:(NSString *)description
                      tags:(NSString *)tags
                    shared:(BOOL)shared
                completion:(LHSDeliciousErrorBlock)completion;

/**
 *  Retrieve a single bookmark.
 *
 *  @param url     The URL for the bookmark to retrieve.
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes two arguments: a dictionary describing the bookmark object that was retrieved and the error (if any) that occurred during the request.
 */

- (void)bookmarkWithURL:(NSString *)url
             completion:(LHSDeliciousDictionaryErrorBlock)completion;

/**
 *  Delete a bookmark.
 *
 *  @param url     The URL for the bookmark to delete.
 *  @param completion  The block to be executed on the completion of an authentication request. The block has no return value and takes one argument: the error (if any) that occurred during the request.
 */
- (void)deleteBookmarkWithURL:(NSString *)url
                   completion:(LHSDeliciousErrorBlock)completion;

#pragma mark Tags

/** @name Tags */

/**
 *  Retrieve all tags.
 *
 *  @param completion A block object to be executed when the request completes successfully. This block has no return value and takes one argument: a dictionary with tag names as keys and frequencies as values.
 */
- (void)tagsWithCompletion:(LHSDeliciousDictionaryBlock)completion;

/**
 *  Delete a tag.
 *
 *  @param tag     The name of the tag to delete.
 *  @param completion A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 */
- (void)deleteTag:(NSString *)tag completion:(LHSDeliciousEmptyBlock)completion;

/**
 *  Rename a tag.
 *
 *  @param oldTag  The current name of the tag.
 *  @param newTag  The updated name of the tag.
 *  @param completion A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 */
- (void)renameTagFrom:(NSString *)oldTag
                   to:(NSString *)newTag
           completion:(LHSDeliciousEmptyBlock)completion;

#pragma mark Tag bundles

/** @name Tag Bundles */

/**
 *  Retrieve all tag bundles.
 *
 *  @param completion A block object to be executed when the request completes successfully. This block has no return value and takes one argument: a dictionary with the names of the tag bundles as keys and consituent tags as values.
 */
- (void)tagBundlesWithCompletion:(LHSDeliciousDictionaryBlock)completion;

/**
 *  Update the tags included in a tag bundle.
 *
 *  @param bundle  The name of the tag bundle to update.
 *  @param tags    The new list of tags to include in the tag bundle.
 *  @param completion A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 */
- (void)updateTagBundle:(NSString *)bundle
               withTags:(NSString *)tags
             completion:(LHSDeliciousEmptyBlock)completion;

/**
 *  Delete a tag bundle.
 *
 *  @param bundle  The name of the tag bundle to delete.
 *  @param completion A block object to be executed when the request completes successfully. This block has no return value and takes no arguments.
 */
- (void)deleteTagBundle:(NSString *)bundle
             completion:(LHSDeliciousEmptyBlock)completion;

@end
