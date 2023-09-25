/*!
@file base64.h
@date Created on: 2020/03/04
@author DATT JAPAN Inc.
@version 3.1
@brief Base64 関連処理
*/

#ifndef base64_h
#define base64_h

#include <stdio.h>

/*!
@name Base64 関連定数定義
@{
*/

/*!
@const BASE64_URL_JPEG_PREFIX
@brief Base64 URL 化した文字列の形式が JPEG であることを示すため先頭に付与する文字列
*/
extern const char *BASE64_URL_PREFIX_JPEG;

/*! @} */


/*!
@brief 文字列を Base64 エンコードし、結果を文字列として返す。
@param srcData エンコード元のバイト配列
@param srcLength エンコード元のバイト配列長
@param dstStr エンコード後の文字列
@return エンコード後の文字列長（NULL 終端を含まない）
*/
size_t base64Encode(const unsigned char *srcData, size_t srcLength, char **dstStr);

/*!
@brief 文字列を Base64 デコードし、結果をバイト配列として返す。
@param srcStr デコード元の文字列
@param srcLength デコード元の文字列長（NULL 終端を含まない）
@param dstData デコード後のバイト配列
@return デコード後のバイト配列の長さ
*/
size_t base64Decode(const char *srcStr, size_t srcLength, unsigned char **dstData);

/*!
@brief SVG から抜き出した Base64 形式の xlink からデータ URL として扱うために必要な接頭辞 `data:[<mediatype>][;base64],` を削除し、純粋な Base64 部分のみを取得する。
@param srcStr NULL 終端されたデータ URL 文字列
@param dstStr NULL 終端された Base64 文字列（srcStr から接頭辞を取り除いた値）
@return dstStr の文字列長（NULL 終端を含まない）
*/
size_t base64WithDataURLPrefixRemoved(const char *srcStr, char **dstStr);

#endif /* base64_h */
