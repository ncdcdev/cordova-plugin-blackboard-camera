/*!
@file base64.c
@date Created on: 2020/03/04
@author DATT JAPAN Inc.
@version 3.1
@brief Base64 関連処理
*/

#include "base64.h"

#include <stdlib.h>
#include <string.h>

/*! Base64 変換テーブルのサイズ */
#define TABLE_SIZE 64

/*! Base64 変換テーブル */
static const char table[TABLE_SIZE] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char *BASE64_URL_PREFIX_JPEG = "data:image/jpeg;base64,";

/*!
@name Base64 エンコード処理
@{
*/

/*!
@brief Base64 エンコード後のデータサイズを返す。
@param srcLength 計算に使用する変換元データの長さ（NULL 終端を含まない）
*/
size_t _getBase64EncodedSize(size_t srcLength)
{
    size_t encodedLength = 0;

    if(srcLength == 0) return 0;

    /* 6 bit 単位のブロック数（6 bit 単位で切り上げ） */
    encodedLength = (srcLength * 8 + 5) / 6;

    /* 4 文字単位のブロック数（4 文字単位で切り上げ） */
    encodedLength = (encodedLength + 3) / 4;

    /* 改行を含まない文字数 */
    encodedLength *= 4;

    return encodedLength;
}

/*!
@brief 文字列を Base64 エンコードし、結果を文字列として返す。
@param srcData エンコード元のバイト配列
@param srcLength エンコード元のバイト配列長
@param dstStr エンコード後の文字列
@return エンコード後の文字列長（NULL 終端を含まない）
*/
size_t base64Encode(const unsigned char *srcData, size_t srcLength, char **dstStr)
{
    int i;
    size_t remain;
    size_t encodedLength;

    if(srcData == NULL) return 0;
    if(dstStr == NULL || *dstStr != NULL) return 0;
    if(srcLength == 0)
    {
        *dstStr = malloc(1);
        *dstStr[0] = '\0';
        return 1;
    }

    remain = srcLength;
    encodedLength = _getBase64EncodedSize(srcLength);

    *dstStr = malloc(encodedLength + 1);
    if(*dstStr == NULL)
    {
        return 0;
    }
    memset(*dstStr, '\0', encodedLength + 1);

    i = 0;

    while(0 < remain)
    {
        if(remain >= 3)
        {
            (*dstStr)[4 * i + 0] = table[ (srcData[3 * i + 0] >> 2) & 0x3f];
            (*dstStr)[4 * i + 1] = table[((srcData[3 * i + 0] << 4) & 0x30) | ((srcData[3 * i + 1] >> 4) & 0xf)];
            (*dstStr)[4 * i + 2] = table[((srcData[3 * i + 1] << 2) & 0x3c) | ((srcData[3 * i + 2] >> 6) & 0x3)];
            (*dstStr)[4 * i + 3] = table[  srcData[3 * i + 2]       & 0x3f];
            remain -= 3;
        }
        else if(remain == 2)
        {
            (*dstStr)[4 * i + 0] = table[ (srcData[3 * i + 0] >> 2) & 0x3f];
            (*dstStr)[4 * i + 1] = table[((srcData[3 * i + 0] << 4) & 0x30) | ((srcData[3 * i + 1] >> 4) & 0xf)];
            (*dstStr)[4 * i + 2] = table[ (srcData[3 * i + 1] << 2) & 0x3c];
            (*dstStr)[4 * i + 3] = '=';
            remain -= 2;
        }
        else /* remain == 1 */
        {
            (*dstStr)[4 * i + 0] = table[ (srcData[3 * i + 0] >> 2) & 0x3f];
            (*dstStr)[4 * i + 1] = table[ (srcData[3 * i + 0] << 4) & 0x30];
            (*dstStr)[4 * i + 2] = '=';
            (*dstStr)[4 * i + 3] = '=';
            remain -= 1;
        }
        i++;
    }

    return encodedLength;
}

/*!
@}

@name Base64 デコード処理
@{
*/

/*!
@brief Base64 デコード後のデータサイズを返す。
@param srcLength 計算に使用する変換元文字列の長さ（NULL 終端を含まない）
*/
size_t _getBase64DecodedSize(const char *data, size_t srcLength)
{
    size_t i;
    size_t decodedLength = 0;
    int equalsCount = 0;

    if(srcLength == 0) return 0;

    /* 4 で割って余りが出る場合は不正なデータ */
    if(srcLength % 4 != 0) return 0;

    /* 末尾の等号の数を数える */
    for(i = 1; i < srcLength; i++)
    {
        if(data[srcLength - i] == '=')
        {
            equalsCount++;
        }
        else
        {
            break;
        }
    }

    /* 末尾の等号は最大 2 個まで */
    if(equalsCount < 0 || 2 < equalsCount)
    {
        return 0;
    }

    /* (バイナリの元サイズ) = ((Base64 のサイズ) / 4 * 3) - (等号の数) */
    decodedLength = (srcLength / 4) * 3 - equalsCount;

    return decodedLength;
}

/*!
@brief 文字列を Base64 デコードし、結果をバイト配列として返す。
@param srcStr デコード元の文字列
@param srcLength デコード元の文字列長（NULL 終端を含まない）
@param dstData デコード後のバイト配列
@return デコード後のバイト配列の長さ
*/
size_t base64Decode(const char *srcStr, size_t srcLength, unsigned char **dstData)
{
    int i, j;
    unsigned char *cursor = NULL;
    char tmp[4];
    size_t remain;
    size_t decodedLength;

    if(srcStr == NULL) return 0;
    if(dstData == NULL || *dstData != NULL) return 0;
    if(srcLength == 0)
    {
        *dstData = malloc(1);
        *dstData[0] = 0x00;
        return 1;
    }

    /* 4 で割って余りが出る場合は不正なデータ */
    if(srcLength % 4 != 0) return 0;

    remain = srcLength;
    decodedLength = _getBase64DecodedSize(srcStr, srcLength);

    *dstData = malloc(decodedLength);
    if(*dstData == NULL)
    {
        return 0;
    }
    memset(*dstData, 0x00, decodedLength);
    cursor = *dstData;

    while(0 < remain)
    {
        for(i = 0; i < 4; i++)
        {
            tmp[i] = 0;

            if('=' != *srcStr)
            {
                for(j = 0; j < TABLE_SIZE; j++)
                {
                    if(table[j] == *srcStr)
                    {
                        tmp[i] = j;
                        break;
                    }
                }
                if(j == TABLE_SIZE)
                {
                    /* Base64 で許容する文字以外を検出した */
                    return 0;
                }
            }
            else
            {
                if(2 < remain) return 0;

                remain = 0;
                break;
            }

            srcStr++;
            remain--;
        }

        switch (i)
        {
        case 4:
            *cursor = ((tmp[0] << 2) & 0xfc) | ((tmp[1] >> 4) & 0x3);
            cursor++;
            *cursor = ((tmp[1] << 4) & 0xf0) | ((tmp[2] >> 2) & 0xf);
            cursor++;
            *cursor = ((tmp[2] << 6) & 0xc0) | ( tmp[3]       & 0x3f);
            cursor++;
            break;

        case 3:
            *cursor = ((tmp[0] << 2) & 0xfc) | ((tmp[1] >> 4) & 0x3);
            cursor++;
            *cursor = ((tmp[1] << 4) & 0xf0) | ((tmp[2] >> 2) & 0xf);
            cursor++;
            break;

        case 2:
            *cursor = ((tmp[0] << 2) & 0xfc) | ((tmp[1] >> 4) & 0x3);
            cursor++;
            break;

        default:
            return 0;
        }
    }

    return decodedLength;
}

/*!
@}

@name その他の Base64 に関する処理
@{
*/

/*!
@brief SVG から抜き出した Base64 形式の xlink からデータ URL として扱うために必要な接頭辞 `data:[<mediatype>][;base64],` を削除し、純粋な Base64 部分のみを取得する。
@param srcStr NULL 終端されたデータ URL 文字列
@param dstStr NULL 終端された Base64 文字列（srcStr から接頭辞を取り除いた値）
@return dstStr の文字列長（NULL 終端を含まない）。処理に失敗した場合は `0`
*/
size_t base64WithDataURLPrefixRemoved(const char *srcStr, char **dstStr)
{
    size_t srcLength, dstLength;
    size_t copyStartPosition;

    if(dstStr == NULL || *dstStr != NULL) return 0;

    srcLength = strlen(srcStr);

    /* strcspn 関数は検索文字の直前のインデックスを指すので +1 */
    copyStartPosition = strcspn(srcStr, ",") + 1;

    if(copyStartPosition >= srcLength)
    {
        return 0;
    }
    dstLength = srcLength - copyStartPosition;

    *dstStr = malloc(dstLength + 1); /* +1 は NULL 終端分 */
    if(*dstStr == NULL) return 0;
    memset(*dstStr, '\0', dstLength + 1);
    strncpy(*dstStr, srcStr + copyStartPosition, dstLength);

    return dstLength;
}

/*! @} */
