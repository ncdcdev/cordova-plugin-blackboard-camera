/*!
@file common.c
@date Created on: 2015/08/18
@author DATT JAPAN Inc.
@version 3.1
@brief 共通処理用ソースコード
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

/*!
@brief 稼働環境のエンディアンを判定し、エンディアンを示す値を返す。
@retval BIG_ENDIAN ビッグエンディアン
@retval LITTLE_ENDIAN リトルエンディアン
*/
JACIC_ENDIANS getEndian(void)
{
    static int i = 1;
    int ret = JACIC_BIG_ENDIAN;

    if(((int)(*(char *)&i)) == 1)
    {
        ret = JACIC_LITTLE_ENDIAN;
    }

    return ret;
}

/*!
@brief 符号なし短整数値をエンディアン変換する。
@param src 符号なし短整数値
@return エンディアン変換後の符号なし短整数値
*/
unsigned short swapEndian16(unsigned short src)
{
    return (src << BIT_SIZE_1BYTE) | ((src >> BIT_SIZE_1BYTE) & 0x00FF);
}

/*!
@brief 符号なし整数値をエンディアン変換する。
@param src 符号なし整数値
@return エンディアン変換後の符号なし整数値
*/
unsigned int swapEndian32(unsigned int src)
{
    return (
               (src) >> (BIT_SIZE_1BYTE * 3) |
               (src & 0x00FF0000) >> (BIT_SIZE_1BYTE * 1) |
               (src & 0x0000FF00) << (BIT_SIZE_1BYTE * 1) |
               (src & 0x000000FF) << (BIT_SIZE_1BYTE * 3)
           );
}

/*!
@brief 数値を 16 進数の文字列表現で取得する。
@param [out] dst 取得対象の文字変数のアドレス
@param hexChar 文字列表現に変換する 16 進数 1 文字で表現可能な数値
@retval FUNCTION_SUCCESS 正常終了
@retval INCORRECT_PARAMETER hexChar が不正である
*/
int getCharCode(unsigned char *dst, unsigned char hexChar)
{
    char tmp[2];

    if(0x0FU < hexChar)
    {
        /* 1 文字表現の範囲外 */
        return INCORRECT_PARAMETER;
    }

    sprintf(tmp, "%x", hexChar);
    *dst = (unsigned char)tmp[0];

    return FUNCTION_SUCCESS;
}

/*!
@brief ファイルにバッファの内容を length バイト書き込む。
@param [in] dst 書き込み対象となるファイルパス
@param [out] outBuff 書き込むバイト配列
@param length 書き込むバイト長
@retval FUNCTION_SUCCESS 正常終了
@retval FILE_OPEN_FAILED ファイルのオープンに失敗
@retval FILE_WRITE_FAILED ファイルへの書き込みに失敗
@retval FILE_CLOSE_FAILED ファイルのクローズに失敗
*/
int writeFile(const char *dst, unsigned char *outBuff, size_t length)
{
    int ret = FUNCTION_SUCCESS;
    int funcRet;
    unsigned long writeResult;
    FILE *fp = NULL;

    fp = fopen(dst, "wb");

    if(fp == NULL)
    {
        /* ファイルのオープンに失敗 */
        return FILE_OPEN_FAILED;
    }

    writeResult = fwrite(outBuff, BYTE_SIZE_UNSIGNED_CHAR, length, fp);

    if(writeResult < length)
    {
        /* ファイルへの書き込みに失敗 */
        ret = FILE_WRITE_FAILED;
    }

    /* ファイルクローズ */
    funcRet = fclose(fp);

    if(funcRet == EOF)
    {
        /* ファイルクローズに失敗 */
        ret = FILE_CLOSE_FAILED;
    }

    fp = NULL;

    return ret;
}

/*!
@brief 実体と長さをもつバイナリデータ構造体のメモリを確保して返す。
@details 確保されたバイナリ領域 _buff は 0x00 で初期化される。
@param dataLen データ部の長さ（binaryData->_len の値に割り当てられる）
@return 確保したバイナリデータ構造体のポインタ。メモリが確保できなかった場合は NULL
*/
struct _binaryData *allocateBinaryData(const size_t dataLen)
{
    size_t memorySize = 0;
    struct _binaryData *binaryData = NULL;

    if(dataLen == 0) return NULL;

    memorySize = sizeof(struct _binaryData) + dataLen;
    binaryData = malloc(memorySize);

    if(binaryData != NULL)
    {
        memset(binaryData, 0x00, memorySize);
        binaryData->_len = dataLen;
    }

    return binaryData;
}

/*!
@brief バイト配列構造体の複製を行う。
@details メモリの動的確保が行われるため、dst を使い終わったら解放する必要がある。
@param [in] src コピー元のバイト配列構造体
@param [out] dst コピー先のバイト配列構造体
*/
void copyBinaryDataToBinaryData(const struct _binaryData *src, struct _binaryData **dst)
{
    if(src == NULL) return;

    copyByteArrayToBinaryData(src->_buff, src->_len, dst);
}

/*!
@brief バイト配列を構造体に複製する。
@details メモリの動的確保が行われるため、dst を使い終わったら解放する必要がある。
@param [in] srcData コピー元のバイト配列
@param srcLength srcData のデータ長
@param [out] dst コピー先のバイト配列構造体
*/
void copyByteArrayToBinaryData(const unsigned char *srcData, const size_t srcLength, struct _binaryData **dst)
{
    if(srcData == NULL) return;
    if(dst == NULL || *dst != NULL) return;
    if(srcLength == 0) return;

    *dst = allocateBinaryData(srcLength);
    if(*dst == NULL) return;

    memcpy((*dst)->_buff, srcData, srcLength);
}
