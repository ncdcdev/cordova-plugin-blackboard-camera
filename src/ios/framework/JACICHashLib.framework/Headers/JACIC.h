//
//  JACIC.h
//  iOSSample
//
//  Created by DATT JAPAN Inc. on 2019/07/12.
//  Copyright © 2019 DATT JAPAN Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface JACIC : NSObject

+ (int)writeHashValueFromSourcePath:(NSString *)sourcePath
                  toDestinationPath:(NSString *)destinationPath
NS_SWIFT_NAME(writeHashValue(from:to:));

@end

NS_ASSUME_NONNULL_END
