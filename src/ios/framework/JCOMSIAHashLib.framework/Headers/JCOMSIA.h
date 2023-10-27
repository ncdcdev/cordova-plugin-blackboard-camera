//
//  JCOMSIA.h
//  iOSSample
//
//  Created by DATT JAPAN Inc. on 2019/07/12.
//  Copyright © 2019 DATT JAPAN Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface JCOMSIA : NSObject

+ (int)writeHashValueFromSourcePath:(NSString *)sourcePath
                  toDestinationPath:(NSString *)destinationPath
NS_SWIFT_NAME(writeHashValue(from:to:));

+ (int)svgCalculateHashValueFromImagePath:(NSString *)imagePath
                           chalkBoardPath:(NSString *)chalkBoardPath
                                 hashCode:(NSString * _Nullable __autoreleasing * _Nullable)hashCode
NS_SWIFT_NAME(svgCalculateHashValue(imagePath:chalkBoardPath:hashCode:));

@end

NS_ASSUME_NONNULL_END
