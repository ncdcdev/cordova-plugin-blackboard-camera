/*!
@file writeHashLib.c
@date Created on: 2015/08/10
@author DATT JAPAN Inc.
@version 3.1
@brief 改ざんチェック用ハッシュ埋め込みモジュールのソースコード
*/

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app1.h"
#include "app5.h"
#include "common.h"
#include "exif.h"
#include "sha256.h"
#include "svg.h"
#include "writeHashLib.h"


/*!
@brief ハッシュ値の長さ
*/
const size_t JCOMSIA_HASH_LENGTH = BYTE_SIZE_HASH_LENGTH;


/* ハッシュ生成に使用するパスワード */
static const char *PASSWORD = "d598ccd39f78df7d900259e9317662f23d8e9e6d32329a695e6e6f2e8cbed4e5";

static HashBuffer *_getPasswordHash()
{
    int ret;
    size_t passwordLength = strlen(PASSWORD);

    HashBuffer *input = NULL;
    HashBuffer *output = NULL;

    input = allocateBinaryData(passwordLength);
    if(input == NULL) return NULL;

    memcpy(input->_buff, PASSWORD, passwordLength);
    output = allocateBinaryData(JCOMSIA_HASH_LENGTH);

    ret = hash(input, output);
    assert(ret == FUNCTION_SUCCESS);

    _SECURE_FREE(input);

    return output;
}

/*!
@brief 指定ファイルの存在チェックを行う。
@param [in] src 検索ファイルパス
@retval FUNCTION_SUCCESS ファイルが存在する
@retval FILE_NOT_EXISTS ファイルが存在しない
*/
int _fileExist(const char *src)
{
    int ret = FUNCTION_SUCCESS;
    FILE *fp = NULL;

    fp = fopen(src, "r");

    if(fp == NULL)
    {
        /* ファイルが存在しない */
        ret = FILE_NOT_EXISTS;

    }
    else
    {
        /* ファイルクローズ */
        int closeResult = fclose(fp);

        if(closeResult == EOF)
        {
            /* ファイルクローズに失敗 */
            ret = FILE_CLOSE_FAILED;
        }

        fp = NULL;

    }

    return ret;
}

/*!
@brief ファイルのサイズを取得する。
@param [out] result ファイルサイズを格納する符号なし長整数
@param [in] fp 検査対象となるファイルのポインタ
@retval FUNCTION_SUCCESS 正常終了
@retval FILE_OPEN_FAILED ファイルオープンに失敗した場合
@retval OTHER_ERROR ファイル読み込み途中でエラーが発生した場合
*/
int _fileSize(size_t *result, FILE *fp)
{
    int ret;

    if(fp == NULL)
    {
        return FILE_OPEN_FAILED;
    }

    /* ファイルサイズ取得 */
    ret = fseek(fp, 0L, SEEK_END);

    if(ret != 0)
    {
        return OTHER_ERROR;
    }

    *result = (size_t)ftell(fp);

    /* ファイルポインタを戻す */
    ret = fseek(fp, 0L, SEEK_SET);

    if(ret != 0)
    {
        return OTHER_ERROR;
    }

    return FUNCTION_SUCCESS;
}

/*!
@brief `filePath` を読み込み、`buffer` に格納する。
@param [in] filePath ファイルの場所
@param [out] buffer 読み込んだ `filePath` のバイナリ情報を格納するためのデータ
@retval FUNCTION_SUCCESS 正常終了
@retval INCORRECT_PARAMETER 不正な引数が指定された場合
@retval FILE_OPEN_FAILED ファイルオープンに失敗した場合
@retval FILE_SIZE_ZERO ファイルサイズが 0 の場合
@retval FILE_CLOSE_FAILED ファイルクローズに失敗した場合
@retval OTHER_ERROR ファイル読み込み途中でエラーが発生した場合、メモリ確保に失敗した場合
*/
int _readFile(const char *filePath, JpegBuffer **buffer)
{
    int ret = FUNCTION_SUCCESS;
    int closeResult;
    size_t fileSize;
    size_t readSize;
    FILE *fp = NULL;

    /* パラメータが不正 */
    if(filePath == NULL || buffer == NULL)
    {
        return INCORRECT_PARAMETER;
    }

    if((fp = fopen(filePath, "rb")) == NULL)
    {
        /* ファイルオープンエラー */
        ret = FILE_OPEN_FAILED;
        goto FINALIZE;
    }

    /* ファイルサイズ取得 */
    if((ret = _fileSize(&fileSize, fp)) != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }

    if(fileSize == 0UL)
    {
        /* ファイルサイズゼロ */
        ret = FILE_SIZE_ZERO;
        goto FINALIZE;
    }

    /* バイナリデータ領域確保 */
    *buffer = allocateBinaryData(fileSize);

    if(buffer == NULL)
    {
        /* メモリ確保失敗 */
        ret = OTHER_ERROR;
        goto FINALIZE;
    }

    /* バイナリデータ読込 */
    readSize = fread((*buffer)->_buff, BYTE_SIZE_UNSIGNED_CHAR, fileSize, fp);

    if(readSize < fileSize)
    {
        /* ファイル読み込み失敗 */
        ret = FILE_OPEN_FAILED;
        goto FINALIZE;
    }

FINALIZE:
    /* ファイルクローズ */
    if(fp != NULL)
    {
        closeResult = fclose(fp);

        if(closeResult == EOF)
        {
            /* ファイルクローズに失敗 */
            ret = FILE_CLOSE_FAILED;
        }

        fp = NULL;
    }

    return ret;
}

/*!
@brief ハッシュを合成して再計算する用途のために結合する。
@param [in] srcBuffer コピー元のデータ構造体
@param [out] dstBuffer コピー先のデータ構造体（メモリ割り当て済み）
@param [in] cursor データのコピー位置を示すポインタ（ dstBuffer のデータの末尾）
*/
void _appendHash(HashBuffer *srcBuffer, HashBuffer *dstBuffer, size_t *cursor)
{
    assert(srcBuffer != NULL);
    assert(dstBuffer != NULL);
    assert(cursor != NULL);

    memcpy(dstBuffer->_buff + *cursor, srcBuffer->_buff, srcBuffer->_len);
    *cursor += srcBuffer->_len;
}

/*!
@brief APP5 領域から取得したハッシュ値と画像データから生成したハッシュ値を比較した結果を返す。
@param [in] app5ImageHash APP5 領域から取得したハッシュ値（画像）
@param [in] dataImageHash 画像データから生成したハッシュ値（画像）
@param [in] app5DateHash APP5 領域から取得したハッシュ値（撮影日時）
@param [in] dataDateHash 画像データから生成したハッシュ値（撮影日時）
@retval SAME_HASH 画像、撮影日時のどちらもハッシュ値が等しい
@retval INCORRECT_HASH_BOTH 画像、撮影日時のどちらもハッシュ値が等しくない
@retval INCORRECT_HASH_IMAGE 画像のハッシュ値が等しくなく、撮影日時のハッシュ値が等しい
@retval INCORRECT_HASH_DATE 画像のハッシュ値が等しく、撮影日時のハッシュ値が等しくない
*/
int _compareHashValues(APP5Item app5ImageHash, HashBuffer *dataImageHash, APP5Item app5DateHash, HashBuffer *dataDateHash)
{
    int imageResult = -1;
    int dateResult  = -1;

    /* APP5領域にいずれのハッシュ値も存在しない */
    if(app5ImageHash.titleExistsFlag == JACIC_BOOL_FALSE &&
            app5DateHash.titleExistsFlag == JACIC_BOOL_FALSE)
    {
        /* ハッシュ値の存在しない画像 */
        return HASH_NOT_EXISTS;
    }

    if(app5ImageHash.value != NULL &&
            dataImageHash != NULL)
    {
        imageResult = memcmp(app5ImageHash.value->_buff, dataImageHash->_buff, JCOMSIA_HASH_LENGTH);
    }

    if(app5DateHash.value != NULL &&
            dataDateHash != NULL)
    {
        dateResult  = memcmp(app5DateHash.value->_buff,  dataDateHash->_buff,  JCOMSIA_HASH_LENGTH);
    }

    if(imageResult == 0 && dateResult == 0)
    {
        /* 画像 : OK , 日時 : OK */
        return SAME_HASH;
    }
    else if(imageResult != 0 && dateResult == 0)
    {
        /* 画像 : NG , 日時 : OK */
        return INCORRECT_HASH_IMAGE;
    }
    else if(imageResult == 0 && dateResult != 0)
    {
        /* 画像 : OK , 日時 : NG */
        return INCORRECT_HASH_DATE;
    }
    else
    {
        /* 画像 : NG , 日時 : NG */
        return INCORRECT_HASH_BOTH;
    }
}

/*!
@brief ハッシュ書き込み関数で内部利用している戻り値定数を外部公開用の定数に置き換える。
@param retval 対象となる戻り値
*/
int _hashWriteReturnValueConvert(int retval)
{
    int ret;

    switch(retval)
    {
    case FUNCTION_SUCCESS:
        ret = JW_SUCCESS;
        break;

    case INCORRECT_PARAMETER:
        ret = JW_ERROR_INCORRECT_PARAMETER;
        break;

    case SAME_FILE_PATH:
        ret = JW_ERROR_SAME_FILE_PATH;
        break;

    case FILE_NOT_EXISTS:
        ret = JW_ERROR_READ_FILE_NOT_EXISTS;
        break;

    case FILE_ALREADY_EXISTS:
        ret = JW_ERROR_WRITE_FILE_ALREADY_EXISTS;
        break;

    case FILE_OPEN_FAILED:
        ret = JW_ERROR_READ_FILE_OPEN_FAILED;
        break;

    case FILE_SIZE_ZERO:
        ret = JW_ERROR_READ_FILE_SIZE_ZERO;
        break;

    case FILE_WRITE_FAILED:
        ret = JW_ERROR_WRITE_FILE_FAILED;
        break;

    case FILE_CLOSE_FAILED:
        ret = JW_ERROR_FILE_CLOSE_FAILED;
        break;

    case INCORRECT_EXIF_FORMAT:
        ret = JW_ERROR_INCORRECT_EXIF_FORMAT;
        break;

    case APP5_ALREADY_EXISTS:
        ret = JW_ERROR_APP5_ALREADY_EXISTS;
        break;

    case DATE_NOT_EXISTS:
        ret = JW_ERROR_DATE_NOT_EXISTS;
        break;

    case OTHER_ERROR:
    default:
        ret = JW_ERROR_OTHER;
        break;
    }

    return ret;
}

/*!
@brief ハッシュチェック関数で内部利用している戻り値定数を外部公開用の定数に置き換える。
@param retval 対象となる戻り値
*/
int _hashCheckReturnValueConvert(int retval)
{
    int ret;

    switch(retval)
    {
    case SAME_HASH:
        ret = JC_OK;
        break;

    case INCORRECT_HASH_BOTH:
        ret = JC_NG;
        break;

    case INCORRECT_HASH_IMAGE:
        ret = JC_NG_IMAGE;
        break;

    case INCORRECT_HASH_DATE:
        ret = JC_NG_DATE;
        break;

    case INCORRECT_PARAMETER:
        ret = JC_ERROR_INCORRECT_PARAMETER;
        break;

    case FILE_NOT_EXISTS:
        ret = JC_ERROR_READ_FILE_NOT_EXISTS;
        break;

    case FILE_OPEN_FAILED:
        ret = JC_ERROR_READ_FILE_OPEN_FAILED;
        break;

    case FILE_SIZE_ZERO:
        ret = JC_ERROR_READ_FILE_SIZE_ZERO;
        break;

    case FILE_CLOSE_FAILED:
        ret = JC_ERROR_FILE_CLOSE_FAILED;
        break;

    case INCORRECT_EXIF_FORMAT:
        ret = JC_ERROR_INCORRECT_EXIF_FORMAT;
        break;

    case APP5_NOT_EXISTS:
        ret = JC_ERROR_APP5_NOT_EXISTS;
        break;

    case INCORRECT_APP5_FORMAT:
        ret = JC_ERROR_INCORRECT_APP5_FORMAT;
        break;

    case HASH_NOT_EXISTS:
        ret = JC_ERROR_HASH_NOT_EXISTS;
        break;

    case DATE_NOT_EXISTS:
        ret = JC_ERROR_DATE_NOT_EXISTS;
        break;

    case OTHER_ERROR:
    default:
        ret = JC_ERROR_OTHER;
        break;
    }

    return ret;
}

/*!
@brief SVG 用ハッシュ作成関数で内部利用している戻り値定数を外部公開用の定数に置き換える。
@param retval 対象となる戻り値
*/
int _createHashReturnValueConvert(int retval)
{
    int ret;

    switch(retval)
    {
    case FUNCTION_SUCCESS:
        ret = JW_HASHER_CREATE_SUCCESS;
        break;

    case INCORRECT_PARAMETER:
        ret = JW_HASHER_ERROR_INCORRECT_PARAMETER;
        break;

    case FILE_NOT_EXISTS:
        ret = JW_HASHER_ERROR_FILE_DOES_NOT_EXIST;
        break;

    case FILE_OPEN_FAILED:
        ret = JW_HASHER_ERROR_FILE_OPEN_FAILED;
        break;

    case FILE_SIZE_ZERO:
        ret = JW_HASHER_ERROR_FILE_SIZE_ZERO;
        break;

    case FILE_CLOSE_FAILED:
        ret = JW_HASHER_ERROR_FILE_CLOSE_FAILED;
        break;

    case ORG_DOES_NOT_HAVE_HASH:
        ret = JW_HASHER_ERROR_ORG_DOES_NOT_HAVE_HASH;
        break;

    case ORG_HASH_NG_IMAGE:
        ret = JW_HASHER_ERROR_ORG_HASH_NG_IMAGE;
        break;

    case ORG_HASH_NG_DATE:
        ret = JW_HASHER_ERROR_ORG_HASH_NG_DATE;
        break;

    case ORG_HASH_NG_BOTH:
        ret = JW_HASHER_ERROR_ORG_HASH_NG_BOTH;
        break;

    case CB_DOES_NOT_HAVE_HASH:
        ret = JW_HASHER_ERROR_CB_DOES_NOT_HAVE_HASH;
        break;

    case CB_HASH_NG_IMAGE:
        ret = JW_HASHER_ERROR_CB_HASH_NG_IMAGE;
        break;

    case CB_HASH_NG_DATE:
        ret = JW_HASHER_ERROR_CB_HASH_NG_DATE;
        break;

    case CB_HASH_NG_BOTH:
        ret = JW_HASHER_ERROR_CB_HASH_NG_BOTH;
        break;

    case OTHER_ERROR:
    default:
        ret = JW_HASHER_ERROR_OTHERS;
        break;
    }

    return ret;
}

/*!
@brief SVG 用ハッシュチェック関数で内部利用している戻り値定数を外部公開用の定数に置き換える。
@param retval 対象となる戻り値
*/
int _svgHashCheckReturnValueConvert(int retval)
{
    int ret;

    switch(retval)
    {
    case SAME_HASH:
        ret = JC_SVG_RESULT_OK;
        break;

    case COMBINATION_HASHES_DOES_NOT_MATCH:
        ret = JC_SVG_RESULT_NG_COMBINATION;
        break;

    case ORG_DOES_NOT_EXIST:
        ret = JC_SVG_ERROR_ORG_DOES_NOT_EXIST;
        break;

    case ORG_DOES_NOT_HAVE_HASH:
        ret = JC_SVG_ERROR_ORG_DOES_NOT_HAVE_HASH;
        break;

    case ORG_HASH_NG_IMAGE:
        ret = JC_SVG_ERROR_NG_ORG_IMAGE;
        break;

    case ORG_HASH_NG_DATE:
        ret = JC_SVG_ERROR_NG_ORG_DATE;
        break;

    case ORG_HASH_NG_BOTH:
        ret = JC_SVG_ERROR_NG_ORG_BOTH;
        break;

    case CB_DOES_NOT_HAVE_HASH:
        ret = JC_SVG_ERROR_CB_DOES_NOT_HAVE_HASH;
        break;

    case CB_HASH_NG_IMAGE:
        ret = JC_SVG_ERROR_NG_CB_IMAGE;
        break;

    case CB_HASH_NG_DATE:
        ret = JC_SVG_ERROR_NG_CB_DATE;
        break;

    case CB_HASH_NG_BOTH:
        ret = JC_SVG_ERROR_NG_CB_BOTH;
        break;

    case INCORRECT_PARAMETER:
        ret = JC_SVG_ERROR_INCORRECT_PARAMETER;
        break;

    case OTHER_ERROR:
    default:
        ret = JC_SVG_ERROR_OTHERS;
        break;
    }

    return ret;
}

/*!
@brief SVG パース処理の結果を公開しているエラーコードに差し替える。
@param svgResult SVG パース処理から返された結果
@return ライブラリとして公開している戻り値を示す int 型の値。
*/
int _svgResultToPublicErrorCode(SVGResult svgResult)
{
    switch(svgResult)
    {
        case SVG_FAILURE_ORIGINAL_IMAGE_DOES_NOT_EXIST:
            return JC_SVG_ERROR_ORG_DOES_NOT_EXIST;

        case SVG_FAILURE_CHALKBOARD_IMAGE_DOES_NOT_EXIST:
            return JC_SVG_ERROR_CB_DOES_NOT_EXIST;

        case SVG_FAILURE_COULD_NOT_READ_ORIGINAL_IMAGE:
            return JC_SVG_ERROR_ORG_DOES_NOT_HAVE_HASH;

        case SVG_FAILURE_COULD_NOT_READ_CHALKBOARD_IMAGE:
            return JC_SVG_ERROR_CB_DOES_NOT_HAVE_HASH;

        case SVG_FAILURE_BROKEN_STRUCTURE:
            return JC_SVG_ERROR_BROKEN_STRUCTURE;

        case SVG_FAILURE_METADATA_BROKEN_STRUCTURE:
            return JC_SVG_ERROR_METADATA_BROKEN_STRUCTURE;

        case SVG_FAILURE_METADATA_INCORRECT_VENDER:
            return JC_SVG_ERROR_METADATA_INCORRECT_VENDER;

        case SVG_FAILURE_METADATA_INCORRECT_SOFTWARE:
            return JC_SVG_ERROR_METADATA_INCORRECT_SOFTWARE;

        case SVG_FAILURE_METADATA_INCORRECT_META_VERSION:
            return JC_SVG_ERROR_METADATA_INCORRECT_FORMAT_VERSION;

        case SVG_FAILURE_METADATA_INCORRECT_STANDARD_VERSION:
            return JC_SVG_ERROR_METADATA_INCORRECT_STANDARD_VERSION;

        case SVG_FAILURE_METADATA_INCORRECT_HASHCODE:
            return JC_SVG_ERROR_METADATA_INCORRECT_HASHCODE;

        default:
            return JC_SVG_ERROR_OTHERS;
    }
}

/*!
@brief 画像データのバイナリからハッシュ値を生成して呼び出し元へ返す。
@param [in] srcBuffer ハッシュ値生成対象の画像バイナリ
@param [out] imageHash 呼び出し元で受け取る画像ハッシュ値（NULL 初期化されていること）
@param [out] dateHash 呼び出し元で受け取る撮影日時ハッシュ値（NULL 初期化されていること）
@retval FUNCTION_SUCCESS 正常終了
@retval INCORRECT_PARAMETER 不正な引数が指定された場合
@retval INCORRECT_EXIF_FORMAT Exif ファイルが不正な形式の場合
@retval DATE_NOT_EXISTS Exif メタデータ内に原画像データの生成日時が見つからなかった場合
@retval OTHER_ERROR メモリの確保に失敗した場合
*/
int _generateHashes(JpegBuffer *srcBuffer, HashBuffer **imageHash, HashBuffer **dateHash)
{
    int ret;
    HashBuffer *imgBuff = NULL;
    HashBuffer *dateBuff = NULL;

    if(srcBuffer == NULL) return INCORRECT_PARAMETER;
    if(srcBuffer->_len == 0) return FILE_SIZE_ZERO;

    if(imageHash == NULL) return INCORRECT_PARAMETER;
    if(*imageHash != NULL) return INCORRECT_PARAMETER;

    if(dateHash == NULL) return INCORRECT_PARAMETER;
    if(*dateHash != NULL) return INCORRECT_PARAMETER;

    /*
     * ハッシュ値計算に必要となる
     * 『画像の圧縮データ開始位置以降のバイナリ』を取得する
     */
    ret = clipCompressedImage(srcBuffer, &imgBuff, JACIC_BOOL_FALSE);
    if(ret != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }

    *imageHash = allocateBinaryData(JCOMSIA_HASH_LENGTH);
    if(*imageHash == NULL)
    {
        /* メモリ確保失敗 */
        ret = OTHER_ERROR;
        goto FINALIZE;
    }

    /* ハッシュ値計算 */
    ret = hash(imgBuff, *imageHash);
    if(ret != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }

    /*
     * APP1 領域からハッシュ値計算に必要となる『撮影日時情報』を取得する
     */
    ret = getDateTimeOriginal(srcBuffer, &dateBuff);
    if(ret != FUNCTION_SUCCESS)
    {
        /* 不正な Exif フォーマット */
        goto FINALIZE;
    }

    *dateHash = allocateBinaryData(JCOMSIA_HASH_LENGTH);
    if(*dateHash == NULL)
    {
        /* メモリ確保失敗 */
        ret = OTHER_ERROR;
        goto FINALIZE;
    }

    /* ハッシュ値計算 */
    ret = hash(dateBuff, *dateHash);
    if(ret != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }

FINALIZE:

    /* メモリ解放 */
    _SECURE_FREE(dateBuff);
    _SECURE_FREE(imgBuff);

    return ret;
}

/*!
@brief 指定されたバイト配列に改ざんチェック値を埋め込んだバイト配列を返す。

@param [in] srcBuff 改ざんチェック値を埋め込む対象のバッファ
@param [out] destBuff 改ざんチェック値を埋めんだ状態を受け付けるバッファ
@retval FUNCTION_SUCCESS 正常終了
@retval INCORRECT_PARAMETER 不正な引数が指定された場合
@retval FILE_SIZE_ZERO バッファのサイズがゼロ
@retval INCORRECT_EXIF_FORMAT Exif ファイルが不正な形式の場合
@retval APP5_ALREADY_EXISTS ignoreApp5Flag が真であり、APP5 領域が既に存在する場合
@retval INCORRECT_EXIF_FORMAT Exif ファイルが不正な形式の場合
@retval APP5_ALREADY_EXISTS APP5 領域が既に存在する場合
@retval DATE_NOT_EXISTS Exif メタデータ内に原画像データの生成日時が見つからなかった場合
@retval OTHER_ERROR メモリの確保に失敗した場合
*/
int _writeHashValueToBuffer(JpegBuffer *srcBuff, JpegBuffer **destBuff)
{
    int ret = FUNCTION_SUCCESS;
    HashBuffer *dateHash = NULL;
    HashBuffer *imageHash = NULL;

    /* パラメータが不正 */
    if(srcBuff == NULL || srcBuff->_len == 0)
    {
        return INCORRECT_PARAMETER;
    }
    if(destBuff == NULL)
    {
        return INCORRECT_PARAMETER;
    }

    if(srcBuff->_len == 0UL)
    {
        /* ファイルサイズゼロ */
        ret = FILE_SIZE_ZERO;
        goto FINALIZE;
    }

    ret = _generateHashes(srcBuff, &imageHash, &dateHash);
    if(ret != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }

    /* 出力 */
    ret = writeApp5ToBuff(srcBuff, imageHash, dateHash, destBuff);

FINALIZE:

    /* メモリ解放 */
    _SECURE_FREE(dateHash);
    _SECURE_FREE(imageHash);

    /* 戻り値を外部公開用の定数に置き換えて返す */
    return _hashWriteReturnValueConvert(ret);
}

/*!
@brief 画像データを読み込み、画像から再計算したハッシュ値と、既に計算して格納しているハッシュ値を比較した結果を返す。
@details 望むなら、再計算したハッシュ値を呼び出し元へ返却する。
@param [in] srcImage 検証対象となる画像データ（改ざん検知情報付与済み）
@param [out] imageHash srcImage から再計算された画像ハッシュ値（NULL初期化されていること）。呼び出し元で不要ならば NULL を渡す。
@param [out] dateHash srcImage から再計算された撮影日時ハッシュ値（NULL初期化されていること）。呼び出し元で不要ならば NULL を渡す。
@retval SAME_HASH ハッシュ値が正しい
@retval INCORRECT_HASH_IMAGE ハッシュ値（画像）が正しくない
@retval INCORRECT_HASH_DATE ハッシュ値（撮影日時）が正しくない
@retval INCORRECT_HASH_BOTH ハッシュ値（画像、撮影日時）が正しくない
@retval INCORRECT_PARAMETER 引数が正しくない
@retval FILE_SIZE_ZERO 読み込んだ画像ファイルのサイズがゼロ
@retval INCORRECT_EXIF_FORMAT 検証対象の画像の Exif フォーマットが不正
@retval APP5_NOT_EXISTS 検証対象の画像から APP5 領域が見つからない
@retval INCORRECT_APP5_FORMAT 検証対象の画像の APP5 領域の記述形式が異なる
@retval HASH_NOT_EXISTS 検証対象の画像にハッシュ値が設定されていない
@retval DATE_NOT_EXISTS 検証対象の画像に日時情報が見つからない
@retval OTHER_ERROR メモリ確保に失敗した場合などその他のエラー
*/
int _validateImage(JpegBuffer *srcImage, HashBuffer **imageHash, HashBuffer **dateHash)
{
    int ret;

    HashBuffer *generatedImageHash = NULL;
    HashBuffer *generatedDateHash = NULL;

    APP5Item app5ImageHash = {JACIC_BOOL_FALSE, NULL};
    APP5Item app5DateHash = {JACIC_BOOL_FALSE, NULL};

    if(srcImage == NULL) return INCORRECT_PARAMETER;
    if(srcImage->_len == 0) return FILE_SIZE_ZERO;

    /* 画像の APP5 セグメント内のハッシュ値を確認、取得 */
    ret = getAPP5HashValue(srcImage, &app5ImageHash, &app5DateHash);
    if(ret != FUNCTION_SUCCESS) goto FINALIZE;

    /* 原本画像から再計算したハッシュ値 2 種類を取得 */
    ret = _generateHashes(srcImage, &generatedImageHash, &generatedDateHash);
    if(ret != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }
    /* 原本画像に埋め込まれているハッシュ値と先ほど計算したハッシュ値が等しいか検証する */
    ret = _compareHashValues(app5ImageHash, generatedImageHash, app5DateHash, generatedDateHash);
    if(ret != SAME_HASH) goto FINALIZE;

    /* 呼び出し元が画像ハッシュの抽出を求めているならば */
    if(imageHash != NULL)
    {
        /* 画像ハッシュをコピーして関数呼び出し元に返す */
        copyBinaryDataToBinaryData(generatedImageHash, imageHash);
        if(*imageHash == NULL)
        {
            ret = OTHER_ERROR;
            goto FINALIZE;
        }
    }

    /* 呼び出し元が撮影日時ハッシュの抽出を求めているならば */
    if(dateHash != NULL)
    {
        /* 撮影日時ハッシュをコピーして関数呼び出し元に返す */
        copyBinaryDataToBinaryData(generatedDateHash, dateHash);
        if(*dateHash == NULL)
        {
            ret = OTHER_ERROR;
            goto FINALIZE;
        }
    }

FINALIZE:
    /* メモリ解放 */
    _SECURE_FREE(generatedImageHash);
    _SECURE_FREE(generatedDateHash);

    _SECURE_FREE(app5ImageHash.value);
    _SECURE_FREE(app5DateHash.value);

    return ret;
}

/*!
@brief 原本画像のバイナリデータと黒板画像のバイナリデータから SVG 画像に埋め込むための改ざん検知情報を計算する。

@param [in] originalImageBuffer 画像改ざん検知情報付与済みの原本画像のデータ
@param [in] chalkboardBuffer 画像改ざん検知情報付与済みの黒板画像のデータ
@param [out] hashCode 計算したハッシュ値を格納するポインタ

@retval FUNCTION_SUCCESS 正常終了

@retval INCORRECT_PARAMETER 引数に NULL を渡された等、引数指定に不備がある場合

@retval ORG_DOES_NOT_HAVE_HASH 原本画像に画像ハッシュまたは撮影日時ハッシュが含まれていない
@retval ORG_HASH_NG_IMAGE 原本画像のハッシュ値（画像）が正しくない
@retval ORG_HASH_NG_DATE 原本画像のハッシュ値（撮影日時）が正しくない
@retval ORG_HASH_NG_BOTH 原本画像のハッシュ値（画像、撮影日時）が正しくない

@retval CB_DOES_NOT_HAVE_HASH 黒板画像に画像ハッシュまたは撮影日時ハッシュが含まれていない
@retval CB_HASH_NG_IMAGE 黒板画像のハッシュ値（画像）が正しくない
@retval CB_HASH_NG_DATE 黒板画像のハッシュ値（撮影日時）が正しくない
@retval CB_HASH_NG_BOTH 黒板画像のハッシュ値（画像、撮影日時）が正しくない

@retval OTHER_ERROR その他のエラー（メモリ領域の確保失敗など）
*/
int _calculateHashValue(JpegBuffer *originalImageBuffer, JpegBuffer *chalkboardBuffer, HashBuffer **hashCode)
{
    int ret;

    size_t cursor;
    size_t combinedHashLength = 0;

    HashBuffer *passwordBuffer = NULL;
    HashBuffer *combinedHash = NULL;

    HashBuffer *originalImageHash = NULL;
    HashBuffer *originalDateHash = NULL;
    HashBuffer *chalkboardImageHash = NULL;
    HashBuffer *chalkboardDateHash = NULL;

    /* パラメータが不正 */
    if(originalImageBuffer == NULL || chalkboardBuffer == NULL || hashCode == NULL)
    {
        return INCORRECT_PARAMETER;
    }

    *hashCode = NULL;

    /* 原本画像に対してハッシュ値の検証を行いながらハッシュ値を取得する */
    ret = _validateImage(originalImageBuffer, &originalImageHash, &originalDateHash);
    if(ret != SAME_HASH)
    {
        /* 一部の戻り値は情報を付け足して返す */
        switch(ret)
        {
            case APP5_NOT_EXISTS:
            case HASH_NOT_EXISTS:
            case INCORRECT_EXIF_FORMAT:
            case INCORRECT_APP5_FORMAT:
                ret = ORG_DOES_NOT_HAVE_HASH;
                break;

            case INCORRECT_HASH_IMAGE:
                ret = ORG_HASH_NG_IMAGE;
                break;

            case INCORRECT_HASH_DATE:
            case INCORRECT_APP1_FORMAT:
            case DATE_NOT_EXISTS:
                ret = ORG_HASH_NG_DATE;
                break;

            case INCORRECT_HASH_BOTH:
                ret = ORG_HASH_NG_BOTH;
                break;
        }
        goto FINALIZE;
    }

    /* 黒板画像に対してハッシュ値の検証を行いながらハッシュ値を取得する */
    ret = _validateImage(chalkboardBuffer, &chalkboardImageHash, &chalkboardDateHash);
    if(ret != SAME_HASH)
    {
        /* 一部の戻り値は情報を付け足して返す */
        switch(ret)
        {
            case APP5_NOT_EXISTS:
            case HASH_NOT_EXISTS:
            case INCORRECT_EXIF_FORMAT:
            case INCORRECT_APP5_FORMAT:
                ret = CB_DOES_NOT_HAVE_HASH;
                break;

            case INCORRECT_HASH_IMAGE:
                ret = CB_HASH_NG_IMAGE;
                break;

            case INCORRECT_HASH_DATE:
            case INCORRECT_APP1_FORMAT:
            case DATE_NOT_EXISTS:
                ret = CB_HASH_NG_DATE;
                break;

            case INCORRECT_HASH_BOTH:
                ret = CB_HASH_NG_BOTH;
                break;
        }
        goto FINALIZE;
    }

    passwordBuffer = _getPasswordHash();

    /* 各種ハッシュ値と“パスワード”を結合した長さを計算 */
    combinedHashLength =
        originalImageHash->_len + originalDateHash->_len +
        chalkboardImageHash->_len + chalkboardDateHash->_len +
        passwordBuffer->_len;

    /* 各種ハッシュ値とパスワードを結合する */
    combinedHash = allocateBinaryData(combinedHashLength);
    if(combinedHash == NULL)
    {
        ret = OTHER_ERROR;
        goto FINALIZE;
    }

    /* 各種ハッシュを結合 */
    cursor = 0;

    /* オリジナル画像のハッシュを結合 */
    _appendHash(originalImageHash,   combinedHash, &cursor);
    _appendHash(originalDateHash,    combinedHash, &cursor);

    /* 黒板画像のハッシュを結合 */
    _appendHash(chalkboardImageHash, combinedHash, &cursor);
    _appendHash(chalkboardDateHash,  combinedHash, &cursor);

    /* パスワードハッシュを結合 */
    _appendHash(passwordBuffer, combinedHash, &cursor);

    *hashCode = allocateBinaryData(JCOMSIA_HASH_LENGTH);
    if(*hashCode == NULL)
    {
        ret = OTHER_ERROR;
        goto FINALIZE;
    }

    /* 結合したハッシュ値の計算 */
    ret = hash(combinedHash, *hashCode);

FINALIZE:

    /* メモリ解放 */
    _SECURE_FREE(originalImageHash);
    _SECURE_FREE(originalDateHash);
    _SECURE_FREE(chalkboardImageHash);
    _SECURE_FREE(chalkboardDateHash);
    _SECURE_FREE(passwordBuffer);
    _SECURE_FREE(combinedHash);

    return ret;
}

/*!
@brief 指定された JPEG ファイルに改ざんチェック値を埋め込んだファイルを出力する。
@details sourceFile と destFile には同じファイルを設定できない。

@param [in] sourceFile 改ざんチェック値を埋め込みたいファイル名
@param [out] destFile 改ざんチェック値を埋め込んだファイル名

@retval JW_SUCCESS                            0 : 正常終了

@retval JW_ERROR_INCORRECT_PARAMETER       -101 : 不正な引数が指定された場合
@retval JW_ERROR_SAME_FILE_PATH            -102 : 読込元と出力先の画像ファイルパスが同じ

@retval JW_ERROR_READ_FILE_NOT_EXISTS      -201 : 読込元画像ファイルが存在しない
@retval JW_ERROR_WRITE_FILE_ALREADY_EXISTS -202 : 出力先画像ファイルが既に存在する
@retval JW_ERROR_READ_FILE_OPEN_FAILED     -203 : ファイルオープン失敗
@retval JW_ERROR_READ_FILE_SIZE_ZERO       -204 : 読み込んだファイルサイズがゼロ
@retval JW_ERROR_WRITE_FILE_FAILED         -205 : ファイル書き込み失敗
@retval JW_ERROR_FILE_CLOSE_FAILED         -206 : ファイルのクローズに失敗

@retval JW_ERROR_INCORRECT_EXIF_FORMAT     -301 : Exif フォーマットが不正
@retval JW_ERROR_APP5_ALREADY_EXISTS       -302 : APP5 セグメントが既に存在する
@retval JW_ERROR_DATE_NOT_EXISTS           -307 : sourceFile に日時情報が見つからない

@retval JW_ERROR_OTHER                     -900 : その他のエラー

@since 1.0
*/
int WINAPI JCOMSIA_WriteHashValue(const char *sourceFile, const char *destFile)
{
    int ret = JW_SUCCESS;
    JpegBuffer *srcJpegBuffer = NULL;
    JpegBuffer *destJpegBuffer = NULL;

    /* パラメータが不正 */
    if(sourceFile == NULL || destFile == NULL)
    {
        return JW_ERROR_INCORRECT_PARAMETER;
    }

    /* 読込元と出力先の画像ファイルパスが同じ場合 */
    if(strcmp(sourceFile, destFile) == 0)
    {
        return JW_ERROR_SAME_FILE_PATH;
    }

    /* 読込元画像ファイルが存在しない場合 */
    if(_fileExist(sourceFile) != FUNCTION_SUCCESS)
    {
        return JW_ERROR_READ_FILE_NOT_EXISTS;
    }

    /* 出力先画像ファイルが既に存在する場合 */
    if(_fileExist(destFile) == FUNCTION_SUCCESS)
    {
        return JW_ERROR_WRITE_FILE_ALREADY_EXISTS;
    }

    /* ファイルオープン */
    ret = _readFile(sourceFile, &srcJpegBuffer);
    if(ret != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }

    /* 出力 */
    /* destBuff は保存対象のバッファ */
    ret = _writeHashValueToBuffer(srcJpegBuffer, &destJpegBuffer);
    if(ret != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }

    ret = writeFile(destFile, destJpegBuffer->_buff, destJpegBuffer->_len);

FINALIZE:

    /* メモリ解放 */
    _SECURE_FREE(destJpegBuffer);
    _SECURE_FREE(srcJpegBuffer);

    /* 戻り値を外部公開用の定数に置き換えて返す */
    return _hashWriteReturnValueConvert(ret);
}

/*!
@brief 指定された JPEG ファイルに正しい改ざんチェック値が設定されているかを確認する。
@param [in] checkFile チェック対象のファイル名

@retval JC_OK                                 1 : ハッシュ値が一致した
@retval JC_NG                                 0 : ハッシュ値（画像、撮影日時の両方）が一致しない
@retval JC_NG_IMAGE                          -1 : ハッシュ値（画像）が一致しない
@retval JC_NG_DATE                           -2 : ハッシュ値（撮影日時）が一致しない

@retval JC_ERROR_INCORRECT_PARAMETER       -101 : 不正な引数が指定された場合

@retval JC_ERROR_READ_FILE_NOT_EXISTS      -201 : 読込元画像ファイルが存在しない
@retval JC_ERROR_READ_FILE_OPEN_FAILED     -203 : ファイルオープン失敗
@retval JC_ERROR_READ_FILE_SIZE_ZERO       -204 : 読み込んだファイルサイズがゼロ
@retval JC_ERROR_FILE_CLOSE_FAILED         -206 : ファイルのクローズに失敗

@retval JC_ERROR_INCORRECT_EXIF_FORMAT     -301 : Exif フォーマットが不正
@retval JC_ERROR_APP5_NOT_EXISTS           -303 : APP5 領域が見つからない
@retval JC_ERROR_INCORRECT_APP5_FORMAT     -304 : APP5 領域の記述形式が異なる
@retval JC_ERROR_HASH_NOT_EXISTS           -305 : ハッシュ値が設定されていない
@retval JC_ERROR_DATE_NOT_EXISTS           -307 : checkFile に日時情報が見つからない

@retval JC_ERROR_OTHER                     -900 : その他のエラー

@since 1.0
*/
int WINAPI JCOMSIA_CheckHashValue(const char *checkFile)
{
    int ret;
    JpegBuffer *readJpegBuffer = NULL;

    /* ファイル読み込みチェック */
    if(checkFile == NULL)
    {
        /* パラメータが不正 */
        return JC_ERROR_INCORRECT_PARAMETER;
    }

    /* チェック対象画像ファイルが存在しない場合 */
    if(_fileExist(checkFile) != FUNCTION_SUCCESS)
    {
        return JC_ERROR_READ_FILE_NOT_EXISTS;
    }

    /* チェック対象画像オープン */
    ret = _readFile(checkFile, &readJpegBuffer);
    if(ret != FUNCTION_SUCCESS)
    {
        goto FINALIZE;
    }

    /* ハッシュ値一致判定 */
    ret = _validateImage(readJpegBuffer, NULL, NULL);

FINALIZE:

    /* メモリ解放 */
    _SECURE_FREE(readJpegBuffer);

    /* 戻り値を外部公開用の定数に置き換えて返す */
    return _hashCheckReturnValueConvert(ret);
}

/*!
@brief 原本画像と黒板画像から SVG 画像に埋め込むための改ざん検知情報を計算する。
@details hashCode は処理が成功した際にメモリ領域が確保されるため、必要に応じて手動で解放する必要がある。
hashCode の長さは `JCOMSIA_HASH_LENGTH` を参照。

@param [in] originalImagePath 画像改ざん検知情報付与済みの原本画像のパス
@param [in] chalkboardPath 画像改ざん検知情報付与済みの黒板画像のパス
@param [out] hashCode 計算したハッシュ値を格納するポインタ。NULL 終端されない。途中で処理が失敗した場合は NULL を返す。

@retval JW_HASHER_CREATE_SUCCESS                   0 : 正常終了

@retval JW_HASHER_ERROR_INCORRECT_PARAMETER     -101 : 引数に NULL を渡された等、引数指定に不備がある場合

@retval JW_HASHER_ERROR_FILE_DOES_NOT_EXIST     -201 : 読み込み元ファイルが存在しない
@retval JW_HASHER_ERROR_FILE_OPEN_FAILED        -203 : ファイルオープン失敗
@retval JW_HASHER_ERROR_FILE_SIZE_ZERO          -204 : 読み込んだファイルサイズがゼロ
@retval JW_HASHER_ERROR_FILE_CLOSE_FAILED       -206 : ファイルクローズ失敗

@retval JW_HASHER_ERROR_ORG_DOES_NOT_HAVE_HASH  -351 : 原本画像に画像ハッシュまたは撮影日時ハッシュが含まれていない
@retval JW_HASHER_ERROR_ORG_HASH_NG_IMAGE       -352 : 原本画像のハッシュ値（画像）が正しくない
@retval JW_HASHER_ERROR_ORG_HASH_NG_DATE        -353 : 原本画像のハッシュ値（撮影日時）が正しくない
@retval JW_HASHER_ERROR_ORG_HASH_NG_BOTH        -354 : 原本画像のハッシュ値（画像、撮影日時）が正しくない

@retval JW_HASHER_ERROR_CB_DOES_NOT_HAVE_HASH   -361 : 黒板画像に画像ハッシュまたは撮影日時ハッシュが含まれていない
@retval JW_HASHER_ERROR_CB_HASH_NG_IMAGE        -362 : 黒板画像のハッシュ値（画像）が正しくない
@retval JW_HASHER_ERROR_CB_HASH_NG_DATE         -363 : 黒板画像のハッシュ値（撮影日時）が正しくない
@retval JW_HASHER_ERROR_CB_HASH_NG_BOTH         -364 : 黒板画像のハッシュ値（画像、撮影日時）が正しくない

@retval JW_HASHER_ERROR_OTHERS                  -900 : その他のエラー（メモリ領域の確保失敗など）

@since 3.0
*/
int WINAPI JCOMSIA_SVG_CalculateHashValue(const char *originalImagePath, const char *chalkboardPath, unsigned char **hashCode)
{
    int ret;

    JpegBuffer *originalImageBuffer = NULL;
    JpegBuffer *chalkBoardBuffer = NULL;

    HashBuffer *returnHashCode = NULL;

    /* パラメータが不正 */
    if(originalImagePath == NULL || chalkboardPath == NULL || hashCode == NULL)
    {
        return JW_HASHER_ERROR_INCORRECT_PARAMETER;
    }

    *hashCode = NULL;

    /* 画像ファイルが存在しない場合 */
    if(_fileExist(originalImagePath) != FUNCTION_SUCCESS ||
            _fileExist(chalkboardPath) != FUNCTION_SUCCESS)
    {
        return JW_HASHER_ERROR_FILE_DOES_NOT_EXIST;
    }

    ret = _readFile(originalImagePath, &originalImageBuffer);
    if(ret != FUNCTION_SUCCESS) goto FINALIZE;

    ret = _readFile(chalkboardPath, &chalkBoardBuffer);
    if(ret != FUNCTION_SUCCESS) goto FINALIZE;

    /* ハッシュ値の算出を行う */
    ret = _calculateHashValue(originalImageBuffer, chalkBoardBuffer, &returnHashCode);
    if(ret != FUNCTION_SUCCESS) goto FINALIZE;

    /* 最終的な結果を受け取る引数にコピーする */
    *hashCode = malloc(returnHashCode->_len);
    if(*hashCode == NULL)
    {
        ret = OTHER_ERROR;
        goto FINALIZE;
    }

    memcpy(*hashCode, returnHashCode->_buff, returnHashCode->_len);

FINALIZE:

    /* メモリ解放 */
    _SECURE_FREE(originalImageBuffer);
    _SECURE_FREE(chalkBoardBuffer);
    _SECURE_FREE(returnHashCode);

    return _createHashReturnValueConvert(ret);
}

/*!
@brief SVG ファイルが改ざんされているものか検証を行う。
@details メタデータ内に含まれる hashCode と、
原本画像 + 黒板画像で再計算したハッシュ値が等しいかを調べる。

もし SVG ファイル内に黒板画像が含まれていない場合、
原本画像単体に対して `JACIC_CheckHashValue` 関数を呼び出したときと同じ値を返す。

@param checkFilePath 処理対象となる SVG ファイルのパス

@retval JC_SVG_RESULT_OK                                   1 : 改ざん無し
@retval JC_SVG_RESULT_NG_COMBINATION                      -3 : 原本画像と黒板画像の組み合わせが正しくない

@retval JC_SVG_ERROR_ORG_DOES_NOT_EXIST                  -21 : SVG内に原本画像が存在しない
@retval JC_SVG_ERROR_ORG_DOES_NOT_HAVE_HASH              -22 : 原本画像からハッシュ値を取得できなかった
@retval JC_SVG_ERROR_NG_ORG_IMAGE                        -23 : 原本画像の画像ハッシュが不正または計算不能
@retval JC_SVG_ERROR_NG_ORG_DATE                         -24 : 原本画像の撮影日時ハッシュが不正または計算不能
@retval JC_SVG_ERROR_NG_ORG_BOTH                         -25 : 原本画像の画像ハッシュおよび撮影日時ハッシュの両方が不正または計算不能

@retval JC_SVG_ERROR_CB_DOES_NOT_EXIST                   -41 : SVG内に合成ハッシュ値が含まれているが、黒板画像が存在しない
@retval JC_SVG_ERROR_CB_DOES_NOT_HAVE_HASH               -42 : 黒板画像からハッシュ値を取得できなかった
@retval JC_SVG_ERROR_NG_CB_IMAGE                         -43 : 黒板画像の画像ハッシュが不正または計算不能
@retval JC_SVG_ERROR_NG_CB_DATE                          -44 : 黒板画像の撮影日時ハッシュが不正または計算不能
@retval JC_SVG_ERROR_NG_CB_BOTH                          -45 : 黒板画像の画像ハッシュおよび撮影日時ハッシュの両方が不正または計算不能

@retval JC_SVG_ERROR_BROKEN_STRUCTURE                    -61 : SVGの構成が壊れている

@retval JC_SVG_ERROR_METADATA_BROKEN_STRUCTURE           -81 : メタデータの構成が壊れている
@retval JC_SVG_ERROR_METADATA_INCORRECT_VENDER           -82 : メタデータ「ベンダー名」に問題あり
@retval JC_SVG_ERROR_METADATA_INCORRECT_SOFTWARE         -83 : メタデータ「ソフトウェア名」に問題あり
@retval JC_SVG_ERROR_METADATA_INCORRECT_FORMAT_VERSION   -84 : メタデータ「メタデータバージョン」に問題あり
@retval JC_SVG_ERROR_METADATA_INCORRECT_STANDARD_VERSION -85 : メタデータ「適用基準バージョン」に問題あり
@retval JC_SVG_ERROR_METADATA_INCORRECT_HASHCODE         -86 : メタデータ「ハッシュコード」に問題あり

@retval JC_SVG_ERROR_INCORRECT_PARAMETER                -100 : NULLなど不正な引数が渡された
@retval JC_SVG_ERROR_OTHERS                             -900 : メモリ確保失敗などの予期せぬエラー

@since 3.0
*/
int WINAPI JCOMSIA_SVG_CheckHashValue(const char *checkFilePath)
{
    int ret, cmpResult;
    SVGResult svgResult;

    JpegBuffer *originalImageBuffer = NULL;
    JpegBuffer *chalkboardBuffer = NULL;
    HashBuffer *svgHashCode = NULL;
    HashBuffer *recalculatedHashCode = NULL;

    /* パラメータが不正 */
    if(checkFilePath == NULL)
    {
        return JC_SVG_ERROR_INCORRECT_PARAMETER;
    }

    /* チェック対象画像ファイルが存在しない場合 */
    if(_fileExist(checkFilePath) != FUNCTION_SUCCESS)
    {
        return JC_ERROR_READ_FILE_NOT_EXISTS;
    }

    /* SVG ファイルの解析 */
    svgResult = parse(checkFilePath, &originalImageBuffer, &chalkboardBuffer, &svgHashCode);
    if(svgResult != SVG_SUCCESS)
    {
        ret = _svgResultToPublicErrorCode(svgResult);
        goto FINALIZE;
    }
    assert(originalImageBuffer != NULL);

    if(chalkboardBuffer == NULL)
    {
        /* 原本画像のみ取得できた場合、原本画像に対して従来のハッシュチェックを行った値を返す */
        ret = _validateImage(originalImageBuffer, NULL, NULL);
        ret = _hashCheckReturnValueConvert(ret);
        goto FINALIZE;
    }

    /* ハッシュ値算出 */
    ret = _calculateHashValue(originalImageBuffer, chalkboardBuffer, &recalculatedHashCode);
    if(ret != FUNCTION_SUCCESS)
    {
        ret = _svgHashCheckReturnValueConvert(ret);
        goto FINALIZE;
    }
    assert(recalculatedHashCode != NULL);

    if(svgHashCode->_len != recalculatedHashCode->_len)
    {
        ret = JC_SVG_RESULT_NG_COMBINATION;
        goto FINALIZE;
    }

    cmpResult = memcmp(svgHashCode->_buff, recalculatedHashCode->_buff, svgHashCode->_len);
    if(cmpResult != 0)
    {
        ret = JC_SVG_RESULT_NG_COMBINATION;
        goto FINALIZE;
    }
    ret = JC_SVG_RESULT_OK;

FINALIZE:

    /* メモリ解放 */
    _SECURE_FREE(originalImageBuffer);
    _SECURE_FREE(chalkboardBuffer);
    _SECURE_FREE(svgHashCode);
    _SECURE_FREE(recalculatedHashCode);

    return ret;
}

/*!
@brief `JCOMSIA_SVG_CalculateHashValue()` 関数によって確保されたメモリ領域を解放する。
@details 解放されたポインタには NULL が設定される。
@attention 動作の安全性のため、本ライブラリによって確保されたメモリ領域以外を指すポインタを引数として与えないこと。

@param [out] ptr メモリ解放の対象となるポインタの参照。処理後は `NULL` ポインタを指すように設定される。

@since 3.1
*/
void WINAPI JCOMSIA_SVG_FreeHashValue(unsigned char **ptr)
{
    if(ptr == NULL) return;

    _SECURE_FREE(*ptr);
}

/*!
@name deprecated
@{
*/

int WINAPI JACIC_WriteHashValue(const char *sourceFile, const char *destFile)
{
    return JCOMSIA_WriteHashValue(sourceFile, destFile);
}

int WINAPI JACIC_CheckHashValue(const char *checkFile)
{
    return JCOMSIA_CheckHashValue(checkFile);
}

int WINAPI JACIC_SVG_CalculateHashValue(const char *originalImagePath, const char *chalkboardPath, unsigned char **hashCode)
{
    return JCOMSIA_SVG_CalculateHashValue(originalImagePath, chalkboardPath, hashCode);
}

int WINAPI JACIC_SVG_CheckHashValue(const char *checkFilePath)
{
    return JCOMSIA_SVG_CheckHashValue(checkFilePath);
}

void WINAPI JACIC_SVG_FreeHashValue(unsigned char **ptr)
{
    JCOMSIA_SVG_FreeHashValue(ptr);
}


/* @} */
