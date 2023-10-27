/*!
@file svg.c
@date Created on: 2020/03/03
@author DATT JAPAN Inc.
@version 3.1
@brief SVG ファイルパース処理
@details
SVG ファイルの解析処理において、XML パーサライブラリ Expat 2.2.9 を使用しています。
ライセンス情報は下記の URL を参照してください。

https://github.com/libexpat/libexpat/blob/master/expat/COPYING
*/

/* Expat を同じライブラリ内のソースとしてビルドする */
#define XML_STATIC

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include "writeHashLib.h"
#include "expat.h"
#include "svg.h"

/*!
@name SVG 解析用定数
@{
*/

static const char * const SVG_TAG_NAME_ROOT                 = "svg";                /*!< @brief SVG ルート要素 */
static const char * const SVG_TAG_NAME_GROUP                = "g";                  /*!< @brief グループ（レイヤー） */
static const char * const SVG_TAG_NAME_IMAGE                = "image";              /*!< @brief 画像 */

static const char * const SVG_TAG_NAME_XMP_ROOT             = "x:xmpmeta";          /*!< @brief XMP データのルート要素 */

static const char * const SVG_TAG_NAME_RDF_ROOT             = "rdf:RDF";            /*!< @brief RDF ドキュメントルート */
static const char * const SVG_TAG_NAME_RDF_DESCRIPTION      = "rdf:Description";    /*!< @brief RDF ドキュメントのデータ要素 */

static const char * const SVG_TAG_NAME_DCPM_VENDER          = "dcpm:vender";        /*!< @brief メタデータのソフトウェアベンダー名格納用タグ */
static const char * const SVG_TAG_NAME_DCPM_SOFTWARE        = "dcpm:software";      /*!< @brief メタデータのソフトウェア名値格納用タグ */
static const char * const SVG_TAG_NAME_DCPM_META_VERSION    = "dcpm:metaVersion";   /*!< @brief メタデータのメタデータバージョン格納用タグ */
static const char * const SVG_TAG_NAME_DCPM_STD_VERSION     = "dcpm:stdVersion";    /*!< @brief メタデータの適用基準バージョン格納用タグ */
static const char * const SVG_TAG_NAME_DCPM_HASHCODE        = "dcpm:hashCode";      /*!< @brief メタデータのハッシュ値格納用タグ */

static const char * const SVG_ATTRIBUTE_NAME_IDENTIFIER     = "id";                 /*!< @brief グループ判定用識別子 */
static const char * const SVG_ATTRIBUTE_NAME_IMAGE_XLINK    = "xlink:href";         /*!< @brief 画像データの格納先（現在は Base64 のデータが埋まっていること前提） */

static const char * const SVG_GROUP_ID_ORIGINAL_IMAGE       = "dcp_org_img";        /*!< @brief 原本画像を示すグループ ID */
static const char * const SVG_GROUP_ID_ANNOTATION           = "dcp_annotation";     /*!< @brief 注釈を示すグループ ID */
static const char * const SVG_GROUP_ID_CHALKBOARD_IMAGE     = "dcp_chalkboard_img"; /*!< @brief 黒板画像を示すグループ ID */

/*! @} */

/*!
@name Expat 用定義
@{
*/

/*! SVG ファイルを読み込む際のバッファサイズ (1MB) */
static const size_t BUFFER_SIZE = (1 * 1024 * 1024);

static void XMLCALL startElementHandler(void *userData, const XML_Char *elementName, const XML_Char **attributes);
static void XMLCALL endElementHandler(void *userData, const XML_Char *elementName);
static void XMLCALL characterDataHandler(void *userData, const XML_Char *data, int len);

/*! @} */

/*!
@name 解析に使用する定義
@{
*/

/*!
@enum NextDataType
@brief 次のデータが何であることを期待して読み取るか。また、どのデータとして扱うかを示す。
@details DATA_NONE の場合は次の値を無視する。
*/
typedef enum
{
    DATA_NONE,              /*!< @brief 任意の属性。処理せず無視する */
    DATA_DCPM_VENDER,       /*!< @brief メタデータのソフトウェアベンダー名 */
    DATA_DCPM_SOFTWARE,     /*!< @brief メタデータのソフトウェア名 */
    DATA_DCPM_META_VERSION, /*!< @brief メタデータのデータバージョン */
    DATA_DCPM_STD_VERSION,  /*!< @brief メタデータの適用基準バージョン */
    DATA_DCPM_HASHCODE      /*!< @brief メタデータのハッシュ値 */
} NextDataType;

/*!
@enum CurrentReadingLayer
@brief 現在読込中のレイヤーが何であるかを示す。
*/
typedef enum
{
    CURRENT_READING_LAYER_NONE,             /*!< @brief なし */
    CURRENT_READING_LAYER_METADATA,         /*!< @brief メタデータ */
    CURRENT_READING_LAYER_ORIGINAL_IMAGE,   /*!< @brief 原本画像レイヤー */
    CURRENT_READING_LAYER_ANNOTATIONS,      /*!< @brief 注釈レイヤー */
    CURRENT_READING_LAYER_CHALKBOARD_IMAGE  /*!< @brief 黒板画像レイヤー */
} CurrentReadingLayer;

/*!
@enum NextExpectedMetaTagType
@brief 次に検出を期待する要素を示す。
*/
typedef enum
{
    NEXT_EXPECTED_TAG_NONE,             /*!< @brief 探査を完全に終了した */

    NEXT_EXPECTED_TAG_SVG,              /*!< @brief SVG 要素のタグを探す */
    NEXT_EXPECTED_TAG_XMP_ROOT,         /*!< @brief XMP 要素のタグを探す */
    NEXT_EXPECTED_TAG_RDF_ROOT,         /*!< @brief RDF ドキュメントの開始タグを探す */
    NEXT_EXPECTED_TAG_RDF_DESCRIPTION,  /*!< @brief RDF ドキュメントデータの開始タグを探す */
    NEXT_EXPECTED_TAG_DCPM_VALUES,      /*!< @brief DCPM メタデータのタグを探す */

    NEXT_EXPECTED_TAG_GROUP,            /*!< @bried グループ要素を探す */
    NEXT_EXPECTED_TAG_IMAGE             /*!< @bried Base64 画像埋め込み要素を探す */
} NextExpectedTagType;

/*!
@enum TagSearchMode
@brief 要素を検出する際のモードを示す。
*/
typedef enum
{
    TAG_SEARCH_MODE_READ_NEXT,          /*!< @brief 特に条件を設けずに次の要素を見る */
    TAG_SEARCH_MODE_SKIP,               /*!< @brief 探している要素に関係ないものはスキップする */
    TAG_SEARCH_MODE_CLOSE_IMMEDIATELY,  /*!< @brief 現在探索中のタグの閉じタグを検出する前に新しいタグを検出した場合はエラーとして扱う */
    TAG_SEARCH_MODE_CAN_NOT_REOPEN_SVG, /*!< @brief 再度 <svg> 要素が開かれた場合にのみエラーとし、それ以外は無視する。 */
} TagSearchMode;

/*!
@struct ParsedResults
@brief 解析の結果取得したデータを保存する構造体。
*/
typedef struct
{
    JpegBuffer *_originalImage;     /*!< @brief 原本画像 */
    JpegBuffer *_chalkboardImage;   /*!< @brief 黒板画像 */
    JACIC_BOOL _hasAnnotation;      /*!< @brief 注釈レイヤを検出したかどうか */

    char *_vender;                  /*!< @brief 作成したソフトウェアベンダー（存在確認のみ） */
    char *_software;                /*!< @brief 作成したソフトウェア名（存在確認のみ） */
    char *_metaVersion;             /*!< @brief メタデータバージョン（現在は 3.1 が正常値） */
    char *_stdVersion;              /*!< @brief 適用基準バージョン（現在は 1.5 が正常値。また、互換性のため 1.0, 0.5 も許容する） */
    HashBuffer *_hashCode;          /*!< @brief ハッシュ値 */

    SVGResult _resultCode;          /*!< @brief エラーがある場合に参照する。 */

} ParsedResults;

/*!
@struct ParseCondition
@brief 解析処理の判定条件を保存する構造体。
*/
typedef struct
{
    XML_Parser _parser;                         /*!< @brief 現在使用中のパーサオブジェクトへの参照を持つポインタ */

    CurrentReadingLayer _currentReadingLayer;   /*!< @brief 現在参照中のレイヤー識別子 */
    NextDataType _nextDataType;                 /*!< @brief 次にデータを読み取った際に期待する値 */
    NextExpectedTagType _nextExpectedTagType;   /*!< @brief タグを読みすすめる際、次に見つけたら内容を確認するタグの種類 */
    TagSearchMode _tagSearchMode;               /*!< @brief タグを読みすすめる際の探し方 */

    size_t _currentTagLevel;                    /*!< @brief 現在読んでいる位置のタグの深さ。ルート要素の中身を 1 とする。 */
    size_t _startsSkippingTagLevel;             /*!< @brief スキップを始めたときのタグの深さ。未設定時は SIZE_T_MAX とする。 */
} ParseCondition;

/*!
@struct ParseInfo
@brief SVG 画像のパース中に利用・保存する情報をまとめた構造体。
@details Expat 関数の userData として扱う。
*/
typedef struct
{
    ParseCondition _condition;  /*!< パース中の条件を示す構造体 */
    ParsedResults _results;     /*!< 解析結果を格納する構造体 */
} ParseInfo;

/*!
@brief ParseInfo 構造体の初期化を行う。
@param parseInfo 初期化対象の構造体
*/
static void _initParseInfo(ParseInfo *parseInfo)
{
    parseInfo->_condition._parser = NULL;
    parseInfo->_condition._currentReadingLayer = CURRENT_READING_LAYER_NONE;
    parseInfo->_condition._nextDataType = DATA_NONE;
    parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_SVG;
    parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_READ_NEXT;

    parseInfo->_condition._currentTagLevel = 0;
    parseInfo->_condition._startsSkippingTagLevel = SIZE_MAX;

    parseInfo->_results._originalImage = NULL;
    parseInfo->_results._chalkboardImage = NULL;
    parseInfo->_results._hasAnnotation = JACIC_BOOL_FALSE;

    parseInfo->_results._vender = NULL;
    parseInfo->_results._software = NULL;
    parseInfo->_results._metaVersion = NULL;
    parseInfo->_results._stdVersion = NULL;
    parseInfo->_results._hashCode = NULL;

    parseInfo->_results._resultCode = SVG_SUCCESS;
}

/*!
@brief ParseInfo 構造体の終了処理を行う。確保しているメモリがある場合は解放する。
@param parseInfo 初期化対象の構造体
*/
static void _deinitParseInfo(ParseInfo *parseInfo)
{
    parseInfo->_condition._parser = NULL;

    _SECURE_FREE(parseInfo->_results._originalImage);
    _SECURE_FREE(parseInfo->_results._chalkboardImage);
    _SECURE_FREE(parseInfo->_results._vender);
    _SECURE_FREE(parseInfo->_results._software);
    _SECURE_FREE(parseInfo->_results._metaVersion);
    _SECURE_FREE(parseInfo->_results._stdVersion);
    _SECURE_FREE(parseInfo->_results._hashCode);
}

/*! @} */

/*!
@name SVG パース用関数
@{
*/

/*!
@brief パース処理を中断し、`parseInfo._resultCode` にエラーコードを設定する。
@param parseInfo [in] パーサーを含むユーザー定義オブジェクト
@param result 設定するエラーコード
*/
void _abortParse(ParseInfo *parseInfo, SVGResult result);

/*!
@brief 属性値を読み取って返す。
@discussion class 属性などスペースによって区切られる値には対応していない。
@param attributeName 取得目的の属性名
@param attributes 文字列として属性名と属性値が交互に入っている検索対象の配列
@return もし attributeName が見つかれば、それに対応した値の文字列。見つからない場合は NULL 。
*/
const XML_Char *_getAttribute(const char *attributeName, const XML_Char **attributes);

/*!
@brief data から長さ len のバイナリをコピーし、len + 1 文字目を NULL にした文字列を新たに返す。
@details malloc 関数によるメモリの動的確保を行っているため、本関数で取得した文字列は使用後に free 関数によるメモリ解放を必要とする。
@retval data の頭から len + 1 の長さの文字列。問題があった場合は NULL 。
*/
char *_copyDataAsCString(const XML_Char *data, int len);

/*!
@brief 必須条件となっているメタデータをすべて取得したかどうか調べる。
@details 適用基準バージョン 1.5 の条件に準拠する。
黒板画像を取得している場合は合成ハッシュ値があるかどうかも確認する。
@return SVG_SUCCESS：必須の要素をすべて取得している場合、その他の値：何らかの取得漏れを検出した場合
*/
SVGResult _isCompletedMetadata(ParsedResults results);

/*!
@brief 取得したバージョン情報が有効なものかどうか調べる。
@details 適用基準バージョン 1.5 の条件に準拠する。
メタデータバージョンは 3.1 固定、適用基準バージョンは 1.5, 1.0, 0.5 のみ許容する。
@return SVG_SUCCESS：すべての条件が正しい場合、その他の値：何らかの値が条件に合わない場合
*/
SVGResult _availableVersion(ParsedResults results);

/*!
@brief メタデータ情報が有効なものかどうか調べる。
@details 適用基準バージョン 1.5 の条件に準拠する。
必須属性の有無、バージョン番号の有効性を検査する。
 @return SVG_SUCCESS：必須の要素をすべて取得している場合、その他の値：何らかのエラーを検出した場合
*/
SVGResult _validateMetadata(ParsedResults results);

/*!
@brief SVG から取得した画像のバイナリデータを Base64 デコードして画像のバイナリデータに変換する。
@param srcStr SVG の <image> 要素内 xlink:href 属性から取得したデータ URL 形式の文字列
@return 変換後の画像データ。変換途中で問題が発生した場合は NULL を返す。
*/
JpegBuffer *_dataURLToImageData(const XML_Char *srcStr);

/*! @} */

/*!
@brief SVG ファイルを解析して各種データを取得する。
@details
オリジナル画像は必ず存在しなければならない。
黒板画像とハッシュ値は場合によっては存在しないことがある。

@param [in] filePath SVG 画像のファイルパス
@param [out] imageBuffer オリジナル画像領域の画像データ
@param [out] chalkboardBuffer 黒板画像領域の画像データ
@param [out] hashBuffer メタデータ領域のハッシュ値

@retval SVG_SUCCESS                                     正常終了

@retval SVG_FAILURE_INCORRECT_PARAMETER                 不正な引数を受け取った
@retval SVG_FAILURE_FILE_CAN_NOT_OPEN                   SVG ファイルを開くことができなかった
@retval SVG_FAILURE_FILE_READ_ERROR                     SVG ファイルの読み込み時にエラーが発生した
@retval SVG_FAILURE_FILE_CLOSE_FAILED                   SVG ファイルのクローズ時にエラーが発生した
@retval SVG_FAILURE_PARSE_ERROR                         SVG ファイルの解析に関して XML パーサからエラーが返された
@retval SVG_FAILURE_BROKEN_STRUCTURE                    SVG の構造が壊れている

@retval SVG_FAILURE_METADATA_BROKEN_STRUCTURE           メタデータの構成が壊れている
@retval SVG_FAILURE_METADATA_INCORRECT_VENDER           「ベンダー名」が正しくない
@retval SVG_FAILURE_METADATA_INCORRECT_SOFTWARE         「ソフトウェア名」が正しくない
@retval SVG_FAILURE_METADATA_INCORRECT_META_VERSION     「メタデータバージョン」が正しくない
@retval SVG_FAILURE_METADATA_INCORRECT_STANDARD_VERSION 「適用基準バージョン」が正しくない
@retval SVG_FAILURE_METADATA_INCORRECT_HASHCODE             「ハッシュコード」が正しくない

@retval SVG_FAILURE_INCORRECT_IMAGE_DETECTED            正しくない画像を検出した

@retval SVG_FAILURE_ORIGINAL_IMAGE_DOES_NOT_EXIST       必須要素の原本画像を取得できなかった
@retval SVG_FAILURE_COULD_NOT_READ_ORIGINAL_IMAGE       原本画像の読み取りに失敗した

@retval SVG_FAILURE_CHALKBOARD_IMAGE_DOES_NOT_EXIST     必須となる条件のうえで黒板画像を取得できなかった
@retval SVG_FAILURE_COULD_NOT_READ_CHALKBOARD_IMAGE     黒板画像の読み取りに失敗した

@retval SVG_FAILURE_OTHER_ERROR                         メモリ確保に失敗したなど、その他のエラー
*/
SVGResult parse(const char *filePath, JpegBuffer **imageBuffer, JpegBuffer **chalkboardBuffer, HashBuffer **hashBuffer)
{
    SVGResult ret = SVG_SUCCESS;
    XML_Bool isFinal;
    void *buffer = NULL;
    FILE *fp = NULL;
    XML_Parser parser;
    ParseInfo parseInfo;

    if(imageBuffer == NULL || *imageBuffer != NULL
            || chalkboardBuffer == NULL || *chalkboardBuffer != NULL
            || hashBuffer == NULL || *hashBuffer != NULL)
    {
        /* 引数不正 */
        return SVG_FAILURE_INCORRECT_PARAMETER;
    }

    if((parser = XML_ParserCreate(NULL)) == NULL)
    {
        return SVG_FAILURE_OTHER_ERROR;
    }

    _initParseInfo(&parseInfo);
    parseInfo._condition._parser = parser;

    XML_SetUserData(parser, &parseInfo);
    XML_SetElementHandler(parser, startElementHandler, endElementHandler);
    XML_SetCharacterDataHandler(parser, characterDataHandler);

    if((fp = fopen(filePath, "rb")) == NULL)
    {
        return SVG_FAILURE_FILE_CAN_NOT_OPEN;
    }

    buffer = malloc(BUFFER_SIZE);
    if(buffer == NULL)
    {
        ret = SVG_FAILURE_OTHER_ERROR;
        goto FINALIZE;
    }

    do
    {
        size_t len;
        enum XML_Status status;

        len = fread(buffer, 1, BUFFER_SIZE, fp);
        if(ferror(fp))
        {
            ret = SVG_FAILURE_FILE_READ_ERROR;
            goto FINALIZE;
        }

        isFinal = len < BUFFER_SIZE;

        status = XML_Parse(parser, buffer, (int)len, isFinal);
        if(status == XML_STATUS_SUSPENDED)
        {
            /* 手動で中断した場合 */
            XML_StopParser(parser, XML_FALSE);
            isFinal = XML_TRUE;
        }
        else if(status == XML_STATUS_ERROR)
        {
            /* エラーで中断した場合 */
#if defined(DEBUG) || defined(_DEBUG) || defined(APP_DEBUG)
            fprintf(stderr, "XML parse error (%lu, %lu)\n",
                    XML_GetCurrentLineNumber(parser),
                    XML_GetCurrentColumnNumber(parser)
                    );
#endif
            XML_StopParser(parser, XML_FALSE);

            if(parseInfo._results._resultCode != SVG_SUCCESS)
            {
                ret = parseInfo._results._resultCode;
            }
            else
            {
#if defined(DEBUG) || defined(_DEBUG) || defined(APP_DEBUG)
            fprintf(stderr, "%s\n", XML_ErrorString(XML_GetErrorCode(parser)));
#endif
                ret = SVG_FAILURE_PARSE_ERROR;
            }

            goto FINALIZE;
        }

    } while(!isFinal);

    /* オリジナル画像は必須 */
    if(parseInfo._results._originalImage != NULL)
    {
        copyBinaryDataToBinaryData(parseInfo._results._originalImage, imageBuffer);
        if(*imageBuffer == NULL)
        {
            ret = SVG_FAILURE_OTHER_ERROR;
            goto FINALIZE;
        }
    }
    else
    {
        ret = SVG_FAILURE_ORIGINAL_IMAGE_DOES_NOT_EXIST;
        goto FINALIZE;
    }

    /* 黒板画像はハッシュコードがない場合のみ任意 */
    if(parseInfo._results._chalkboardImage != NULL)
    {
        copyBinaryDataToBinaryData(parseInfo._results._chalkboardImage, chalkboardBuffer);
        if(*chalkboardBuffer == NULL)
        {
            ret = SVG_FAILURE_OTHER_ERROR;
            goto FINALIZE;
        }
    }
    else if(parseInfo._results._hashCode != NULL)
    {
        ret = SVG_FAILURE_CHALKBOARD_IMAGE_DOES_NOT_EXIST;
        goto FINALIZE;
    }

    /* ハッシュ値は両方取得できたときしか使えないので、それ以外の場合はコピーしない */
    if(parseInfo._results._originalImage != NULL &&
            parseInfo._results._chalkboardImage != NULL &&
            parseInfo._results._hashCode != NULL)
    {
        copyBinaryDataToBinaryData(parseInfo._results._hashCode, hashBuffer);
        if(*hashBuffer == NULL)
        {
            ret = SVG_FAILURE_OTHER_ERROR;
            goto FINALIZE;
        }
    }

FINALIZE:
    /* メモリ解放 */

    _deinitParseInfo(&parseInfo);

    _SECURE_FREE(buffer);

    /* ファイルクローズ */
    if(fp != NULL)
    {
        if(fclose(fp) == EOF)
        {
            /* ファイルクローズに失敗 */
            ret = SVG_FAILURE_FILE_CLOSE_FAILED;
        }

        fp = NULL;
    }

    XML_ParserFree(parser);
    parser = NULL;

    return ret;
}

/*!
@brief XML 解析中、タグの開始部分に到達した場合に呼び出される。
@details 属性値はタグの開始部分に含まれるため、ここで属性値をすべて読み取る。
Expat のパーサに渡され、フレームワーク内部からコールされる。
*/
static void XMLCALL startElementHandler(void *userData, const XML_Char *elementName, const XML_Char **attributes)
{
    ParseInfo *parseInfo = userData;

    NextExpectedTagType nextExpectedTag = parseInfo->_condition._nextExpectedTagType;
    TagSearchMode searchMode = parseInfo->_condition._tagSearchMode;

    (parseInfo->_condition._currentTagLevel)++;

    if(searchMode == TAG_SEARCH_MODE_CAN_NOT_REOPEN_SVG)
    {
        /* すでに <svg> 要素の探索を終了している場合 */
        if(strcmp(elementName, SVG_TAG_NAME_ROOT) == 0) {
            /* <svg> */

            /* SVG 工事写真の仕様として、<svg> 要素は 1 ファイルに 1 個しか入っていないものとする */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }

        /* <svg> 要素ではない場合、描画要素ではないとみなして無視する */
        return;
    }

    if(searchMode == TAG_SEARCH_MODE_SKIP)
    {
        /* 要素読み込みスキップ中の場合 */
        /* スキップ終了条件は EndElement 側で判定しているため、ここでは何もしない */
        return;
    }
    else if(searchMode == TAG_SEARCH_MODE_CLOSE_IMMEDIATELY)
    {
        /* 他の要素を検出しないことを期待する場合、要素が開かれたら不正 */

        SVGResult errorCode;

        if(parseInfo->_condition._currentReadingLayer == CURRENT_READING_LAYER_METADATA)
        {
            errorCode = SVG_FAILURE_METADATA_BROKEN_STRUCTURE;
        }
        else
        {
            errorCode = SVG_FAILURE_BROKEN_STRUCTURE;
        }
        _abortParse(parseInfo, errorCode);
        return;
    }

    if(nextExpectedTag == NEXT_EXPECTED_TAG_SVG)
    {
        /* <svg> */

        if(strcmp(elementName, SVG_TAG_NAME_ROOT) == 0)
        {
            /*
             <svg> 要素直下にメタデータ要素を期待する。
             それ以外の場合はメタデータ要素が正常な位置にないか存在しないかのいずれかとなるためエラーとする。
             */
            parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_XMP_ROOT;
        }

        /* ルート要素が <xml> と出力するアプリもあるため、ルート要素は <svg> 以外も許容する。 */
    }
    else if(nextExpectedTag == NEXT_EXPECTED_TAG_XMP_ROOT)
    {
        /* <x:xmpmeta> */

        if(strcmp(elementName, SVG_TAG_NAME_XMP_ROOT) == 0)
        {
            /*
             <x:xmpmeta> 要素直下に <rdf:RDF> 要素があることを期待する。
             それ以外の場合はメタタグの構成が正しくないためエラーとする。
             */
            parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_RDF_ROOT;
            parseInfo->_condition._currentReadingLayer = CURRENT_READING_LAYER_METADATA;
        }
        else
        {
            /* <svg> 直下の最初の要素に <x:xmpmeta> が来ていない */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }
    }
    else if(nextExpectedTag == NEXT_EXPECTED_TAG_RDF_ROOT)
    {
        /* <rdf:RDF> */

        if(strcmp(elementName, SVG_TAG_NAME_RDF_ROOT) == 0)
        {
            /* <rdf:RDF> 要素以下に <rdf:Description> タグを期待する。 */
            parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_RDF_DESCRIPTION;
        }
        else
        {
            /* <x:xmpmeta> 直下の最初の要素に <rdf:RDF> 要素が来ていない */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }
    }
    else if(nextExpectedTag == NEXT_EXPECTED_TAG_RDF_DESCRIPTION)
    {
        /* <rdf:Description> */

        if(strcmp(elementName, SVG_TAG_NAME_RDF_DESCRIPTION) == 0)
        {
            /* <rdf:Description> 要素以下に dcpm メタデータを期待する。 */
            parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_DCPM_VALUES;
            parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_READ_NEXT;
        }
        else
        {
            /*
             XMP の場合、トップレベル要素は <rdf:Description> 要素でなければならないと定められているため、
             この要素を期待しているときに他の要素を検出した場合はエラーとする
             */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }
    }
    else if(nextExpectedTag == NEXT_EXPECTED_TAG_DCPM_VALUES)
    {
        /* dcpm メタデータ */

        if(strcmp(elementName, SVG_TAG_NAME_DCPM_VENDER) == 0)
        {
            if(parseInfo->_results._vender != NULL)
            {
                /* 同じタグを複数個検出した */
                _abortParse(parseInfo, SVG_FAILURE_METADATA_INCORRECT_VENDER);
                return;
            }

            parseInfo->_condition._nextDataType = DATA_DCPM_VENDER;
        }
        else if(strcmp(elementName, SVG_TAG_NAME_DCPM_SOFTWARE) == 0)
        {
            if(parseInfo->_results._software != NULL)
            {
                /* 同じタグを複数個検出した */
                _abortParse(parseInfo, SVG_FAILURE_METADATA_INCORRECT_SOFTWARE);
                return;
            }

            parseInfo->_condition._nextDataType = DATA_DCPM_SOFTWARE;
        }
        else if(strcmp(elementName, SVG_TAG_NAME_DCPM_META_VERSION) == 0)
        {
            if(parseInfo->_results._metaVersion != NULL)
            {
                /* 同じタグを複数個検出した */
                _abortParse(parseInfo, SVG_FAILURE_METADATA_INCORRECT_META_VERSION);
                return;
            }

            parseInfo->_condition._nextDataType = DATA_DCPM_META_VERSION;
        }
        else if(strcmp(elementName, SVG_TAG_NAME_DCPM_STD_VERSION) == 0)
        {
            if(parseInfo->_results._stdVersion != NULL)
            {
                /* 同じタグを複数個検出した */
                _abortParse(parseInfo, SVG_FAILURE_METADATA_INCORRECT_STANDARD_VERSION);
                return;
            }

            parseInfo->_condition._nextDataType = DATA_DCPM_STD_VERSION;
        }
        else if(strcmp(elementName, SVG_TAG_NAME_DCPM_HASHCODE) == 0)
        {
            if(parseInfo->_results._hashCode != NULL)
            {
                /* 同じタグを複数個検出した */
                _abortParse(parseInfo, SVG_FAILURE_METADATA_INCORRECT_HASHCODE);
                return;
            }

            parseInfo->_condition._nextDataType = DATA_DCPM_HASHCODE;
        }
        else
        {
            /* それ以外のタグが入っている場合、無視して次のデータを探す */
            parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_READ_NEXT;

            /* 無関係のタグが含まれている場合に別のデータを取得する可能性があるため、取得対象のリセットを行う */
            parseInfo->_condition._nextDataType = DATA_NONE;
        }
    }
    else if(nextExpectedTag == NEXT_EXPECTED_TAG_GROUP)
    {
        /* グループ要素（レイヤ） */

        const XML_Char *id = NULL;

        if(strcmp(elementName, SVG_TAG_NAME_GROUP) != 0)
        {
            /* 別の描画要素その他が含まれているため、SVG 工事写真としては構成エラーとする */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }

        id = _getAttribute(SVG_ATTRIBUTE_NAME_IDENTIFIER, attributes);

        if(id == NULL)
        {
            /* レイヤに相当するグループの id が無い = 基準に則っていない */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }

        if(strcmp(id, SVG_GROUP_ID_ORIGINAL_IMAGE) == 0)
        {
            /* 原本画像レイヤ */

            if(parseInfo->_results._originalImage != NULL)
            {
                /* 既に原本画像レイヤを検出済みの場合は構成エラー */
                _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
                return;
            }

            parseInfo->_condition._currentReadingLayer = CURRENT_READING_LAYER_ORIGINAL_IMAGE;
            parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_IMAGE;

        }
        else if(strcmp(id, SVG_GROUP_ID_ANNOTATION) == 0)
        {
            /* 注釈レイヤ */

            if(parseInfo->_results._originalImage == NULL)
            {
                /*
                 原本画像レイヤは必須項目かつ注釈レイヤより前に来る必要があるため、
                 この時点で見つかっていない場合は構成エラー
                 */
                _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
                return;
            }

            if(parseInfo->_results._chalkboardImage != NULL)
            {
                /*
                 注釈レイヤは任意項目だが、黒板画像レイヤより前に来る必要がある
                 そのためこの時点で黒板レイヤを検出済みの場合は構成エラー
                 */
                _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
                return;
            }

            if(parseInfo->_results._hasAnnotation == JACIC_BOOL_TRUE)
            {
                /* 既に注釈レイヤを検出済みの場合は構成エラー */
                _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
                return;
            }

            /* 注釈レイヤは存在だけ確認するものとし、内容について触れない */
            parseInfo->_results._hasAnnotation = JACIC_BOOL_TRUE;

            /* ひとつ上のグループ要素の終了まで読み取り処理をスキップする */
            parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_SKIP;
            parseInfo->_condition._startsSkippingTagLevel = parseInfo->_condition._currentTagLevel - 1;
        }
        else if(strcmp(id, SVG_GROUP_ID_CHALKBOARD_IMAGE) == 0)
        {
            /* 黒板画像レイヤ */

            if(parseInfo->_results._originalImage == NULL)
            {
                /*
                 原本画像は必須項目かつ黒板画像レイヤより前に来る必要があるため、
                 この時点で見つかっていない場合は構成エラー
                 */
                _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
                return;
            }

            if(parseInfo->_results._hashCode == NULL ||
                    parseInfo->_results._hashCode->_len == 0)
            {
                /* メタデータのハッシュコードが空欄で黒板画像レイヤが存在する場合 */
                _abortParse(parseInfo, SVG_FAILURE_METADATA_INCORRECT_HASHCODE);
                return;
            }

            if(parseInfo->_results._chalkboardImage != NULL)
            {
                /* 既に黒板画像レイヤを検出済みの場合は構成エラー */
                _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
                return;
            }

            parseInfo->_condition._currentReadingLayer = CURRENT_READING_LAYER_CHALKBOARD_IMAGE;
            parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_IMAGE;
        }
        else
        {
            /* 上記以外のレイヤは現在の基準で認められていないため構成エラー */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }
    }
    else if(nextExpectedTag == NEXT_EXPECTED_TAG_IMAGE)
    {
        /* 画像要素 */

        JpegBuffer *decodedImage;
        const char *base64URLImage = _getAttribute(SVG_ATTRIBUTE_NAME_IMAGE_XLINK, attributes);

        switch(parseInfo->_condition._currentReadingLayer)
        {
            case CURRENT_READING_LAYER_ORIGINAL_IMAGE:
                if(base64URLImage == NULL || strlen(base64URLImage) == 0)
                {
                    _abortParse(parseInfo, SVG_FAILURE_ORIGINAL_IMAGE_DOES_NOT_EXIST);
                    return;
                }

                if(strncmp(base64URLImage, BASE64_URL_PREFIX_JPEG, strlen(BASE64_URL_PREFIX_JPEG)) != 0)
                {
                    _abortParse(parseInfo, SVG_FAILURE_INCORRECT_IMAGE_DETECTED);
                    return;
                }

                decodedImage = _dataURLToImageData(base64URLImage);

                if(decodedImage == NULL)
                {
                    _abortParse(parseInfo, SVG_FAILURE_COULD_NOT_READ_ORIGINAL_IMAGE);
                    return;
                }

                parseInfo->_results._originalImage = decodedImage;
                parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_GROUP;
                break;

            case CURRENT_READING_LAYER_CHALKBOARD_IMAGE:
                if(base64URLImage == NULL || strlen(base64URLImage) == 0)
                {
                    _abortParse(parseInfo, SVG_FAILURE_CHALKBOARD_IMAGE_DOES_NOT_EXIST);
                    return;
                }

                if(strncmp(base64URLImage, BASE64_URL_PREFIX_JPEG, strlen(BASE64_URL_PREFIX_JPEG)) != 0)
                {
                    _abortParse(parseInfo, SVG_FAILURE_INCORRECT_IMAGE_DETECTED);
                    return;
                }

                decodedImage = _dataURLToImageData(base64URLImage);

                if(decodedImage == NULL)
                {
                    _abortParse(parseInfo, SVG_FAILURE_COULD_NOT_READ_CHALKBOARD_IMAGE);
                    return;
                }

                parseInfo->_results._chalkboardImage = decodedImage;
                parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_NONE;
                break;

            default:
                /* 不正な組み合わせでここに到達した */
                _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
                return;
        }

        /* 画像解析を行ったあと、すぐにグループが閉じられることを期待する。 */
        parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_CLOSE_IMMEDIATELY;
    }
}

/*!
@brief XML 解析中、タグの終了部分に到達した場合に呼び出される。
@details Expat のパーサに渡され、フレームワーク内部からコールされる。
*/
static void XMLCALL endElementHandler(void *userData, const XML_Char *elementName)
{
    ParseInfo *parseInfo = userData;
    TagSearchMode searchMode = parseInfo->_condition._tagSearchMode;
    NextExpectedTagType nextEcpectedTag = parseInfo->_condition._nextExpectedTagType;

    (parseInfo->_condition._currentTagLevel)--;

    if(nextEcpectedTag == NEXT_EXPECTED_TAG_NONE)
    {
        /* 探索終了済みの場合は無視する（不正な構成を検出する目的で読み進めているため中断はしない） */
        return;
    }

    if(searchMode == TAG_SEARCH_MODE_SKIP)
    {
        /* 特定の要素が閉じられるまで読み飛ばす処理 */
        if(parseInfo->_condition._startsSkippingTagLevel < parseInfo->_condition._currentTagLevel)
        {
            /* スキップを継続する */
            return;
        }

        /* スキップを開始したタグまで閉じたことを確認した */
        parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_READ_NEXT;
        parseInfo->_condition._startsSkippingTagLevel = SIZE_MAX;
        return;
    }

    if(strcmp(elementName, SVG_TAG_NAME_GROUP) == 0)
    {
        /* 処理中レイヤ条件をリセットして続行 */
        parseInfo->_condition._currentReadingLayer = CURRENT_READING_LAYER_NONE;
    }

    if(searchMode == TAG_SEARCH_MODE_CLOSE_IMMEDIATELY)
    {
        /* すぐに閉じられることを期待しているので正常 */
        parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_READ_NEXT;
        return;
    }

    /* タグごとの処理 */
    if(strcmp(elementName, SVG_TAG_NAME_ROOT) == 0)
    {
        /* </svg> */

        SVGResult metadataValidateResult;

        if(nextEcpectedTag == NEXT_EXPECTED_TAG_XMP_ROOT)
        {
            /* メタデータを見つける前に終了 */
            /* 何も入っていない空の SVG ファイル */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }

        metadataValidateResult = _validateMetadata(parseInfo->_results);

        if(metadataValidateResult != SVG_SUCCESS)
        {
            /* メタデータに異常あり */
            _abortParse(parseInfo, metadataValidateResult);
            return;
        }

        /* すべての探索を終了する */
        parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_NONE;
        parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_CAN_NOT_REOPEN_SVG;
    }
    else if(strcmp(elementName, SVG_TAG_NAME_XMP_ROOT) == 0)
    {
        /* </x:xmpmeta> */

        SVGResult metadataValidateResult;

        if(nextEcpectedTag == NEXT_EXPECTED_TAG_RDF_ROOT)
        {
            /* 空の <x:xmpmeta> 要素 */
            _abortParse(parseInfo, SVG_FAILURE_METADATA_BROKEN_STRUCTURE);
            return;
        }

        metadataValidateResult = _isCompletedMetadata(parseInfo->_results);

        if(metadataValidateResult != SVG_SUCCESS)
        {
            /* すべてのメタデータを検出する前にメタデータ部の終わりに到達した */
            _abortParse(parseInfo, metadataValidateResult);
            return;
        }

        parseInfo->_condition._currentReadingLayer = CURRENT_READING_LAYER_NONE;
    }
    else if(strcmp(elementName, SVG_TAG_NAME_RDF_ROOT) == 0)
    {
        /* </rdf:RDF> */

        // <rdf:RDF> 終了時はメタデータが揃っているかどうかだけを見る
        if(_isCompletedMetadata(parseInfo->_results) != SVG_SUCCESS)
        {
            /* すべてのメタデータを検出する前にメタデータ部の終わりに到達した */
            _abortParse(parseInfo, SVG_FAILURE_METADATA_BROKEN_STRUCTURE);
            return;
        }

        /*
         XMP の場合、ルートとなる要素 <x:xmpmeta> 直下には
         <rdf:RDF> タグ 1 件のみと定められているため、
         すぐに次の閉じタグが来ることを期待し、それ以外は許容しない。
         */
        parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_CLOSE_IMMEDIATELY;
        parseInfo->_condition._nextExpectedTagType = NEXT_EXPECTED_TAG_GROUP;
    }
    else if(nextEcpectedTag == NEXT_EXPECTED_TAG_DCPM_VALUES)
    {
        /* メタデータ部 */

        /* 無関係のタグが含まれている場合に別のデータを取得する可能性があるため、取得対象のリセットを行う */
        parseInfo->_condition._nextDataType = DATA_NONE;
    }
    else if(strcmp(elementName, SVG_TAG_NAME_GROUP) == 0)
    {
        /* </group> */
        if(nextEcpectedTag == NEXT_EXPECTED_TAG_IMAGE)
        {
            /* レイヤ内の画像を検出する前にタグを閉じている */
            _abortParse(parseInfo, SVG_FAILURE_BROKEN_STRUCTURE);
            return;
        }
    }
    else if(strcmp(elementName, SVG_TAG_NAME_IMAGE) == 0)
    {
        /* <image /> */
        if(parseInfo->_condition._currentReadingLayer == CURRENT_READING_LAYER_ORIGINAL_IMAGE ||
                parseInfo->_condition._currentReadingLayer == CURRENT_READING_LAYER_CHALKBOARD_IMAGE)
        {
            /* 各画像レイヤでは image タグ 1 個のみが含まれており、それ以外の要素は許容しない。 */
            parseInfo->_condition._tagSearchMode = TAG_SEARCH_MODE_CLOSE_IMMEDIATELY;
        }
    }
}

/*!
@brief XML 解析中、文字列値に到達した場合に呼び出される。
@details Expat のパーサに渡され、フレームワーク内部からコールされる。
*/
static void XMLCALL characterDataHandler(void *userData, const XML_Char *data, int len)
{
    ParseInfo *parseInfo = userData;

    if(parseInfo == NULL ||
       parseInfo->_condition._nextDataType == DATA_NONE ||
       parseInfo->_condition._nextExpectedTagType == NEXT_EXPECTED_TAG_NONE) return;

    /* 現状データ部が必要なタグは DCPM メタデータ部のみ。 */
    switch(parseInfo->_condition._nextDataType)
    {
        case DATA_DCPM_VENDER: /* ベンダー名 */
            /* 存在確認のみ（文字列で受け取る） */
            parseInfo->_results._vender = _copyDataAsCString(data, len);
            parseInfo->_condition._nextDataType = DATA_NONE;
            break;

        case DATA_DCPM_SOFTWARE: /* ソフトウェア名 */
            /* 存在確認のみ（文字列で受け取る） */
            parseInfo->_results._software = _copyDataAsCString(data, len);
            parseInfo->_condition._nextDataType = DATA_NONE;
            break;

        case DATA_DCPM_META_VERSION: /* メタデータバージョン */
            /* 文字列として使用する */
            parseInfo->_results._metaVersion = _copyDataAsCString(data, len);
            parseInfo->_condition._nextDataType = DATA_NONE;
            break;

        case DATA_DCPM_STD_VERSION: /* 適用基準バージョン */
            /* 文字列として使用する */
            parseInfo->_results._stdVersion = _copyDataAsCString(data, len);
            parseInfo->_condition._nextDataType = DATA_NONE;
            break;

        case DATA_DCPM_HASHCODE: /* ハッシュコード */
            /* 検証に使用するのでバイナリデータ構造体で受け取る */
            copyByteArrayToBinaryData((const unsigned char *)data, len, &(parseInfo->_results._hashCode));
            parseInfo->_condition._nextDataType = DATA_NONE;
            break;

        case DATA_NONE:
        default:
            break;
    }
}

/*!
@name SVG パース用関数（実装）
@{
*/

void _abortParse(ParseInfo *parseInfo, SVGResult result)
{
    parseInfo->_results._resultCode = result;
    XML_StopParser(parseInfo->_condition._parser, XML_FALSE);
}

const XML_Char *_getAttribute(const char *attributeName, const XML_Char **attributes)
{
    int count = 0;
    const XML_Char *attribute;

    while((attribute = attributes[count]) != NULL)
    {
        if(strcmp(attributeName, attribute) == 0)
        {
            return attributes[count + 1];
        }

        count += 2;
    }

    return NULL;
}

char *_copyDataAsCString(const XML_Char *data, int len)
{
    char *value = malloc(len + 1);

    if(value != NULL)
    {
        memset(value, '\0', len + 1);
        memcpy(value, data, len);
    }

    return value;
}

SVGResult _isCompletedMetadata(ParsedResults results)
{
    if(results._vender == NULL)
    {
        return SVG_FAILURE_METADATA_INCORRECT_VENDER;
    }
    else if(results._software == NULL)
    {
        return SVG_FAILURE_METADATA_INCORRECT_SOFTWARE;
    }
    else if(results._metaVersion == NULL)
    {
        return SVG_FAILURE_METADATA_INCORRECT_META_VERSION;
    }
    else if(results._stdVersion == NULL)
    {
        return SVG_FAILURE_METADATA_INCORRECT_STANDARD_VERSION;
    }
    else if(results._chalkboardImage != NULL && results._hashCode == NULL)
    {
        return SVG_FAILURE_METADATA_INCORRECT_HASHCODE;
    }

    return SVG_SUCCESS;
}

SVGResult _availableVersion(ParsedResults results)
{
    if(strcmp(results._metaVersion, "3.1") != 0)
    {
        return SVG_FAILURE_METADATA_INCORRECT_META_VERSION;
    }
    else if(strcmp(results._stdVersion, "1.5") != 0 &&
            strcmp(results._stdVersion, "1.0") != 0 &&
            strcmp(results._stdVersion, "0.5") != 0)
    {
        return SVG_FAILURE_METADATA_INCORRECT_STANDARD_VERSION;
    }

    return SVG_SUCCESS;
}

SVGResult _validateMetadata(ParsedResults results)
{
    SVGResult retval;

    /* すべての必須情報を取得できているか確認する */
    retval = _isCompletedMetadata(results);
    if(retval != SVG_SUCCESS) return retval;

    /* このライブラリで判定可能なバージョンのみ OK として扱う */
    retval = _availableVersion(results);
    if(retval != SVG_SUCCESS) return retval;

    return SVG_SUCCESS;
}

JpegBuffer *_dataURLToImageData(const XML_Char *srcStr)
{
    char *base64Str = NULL;
    unsigned char *imageData = NULL;
    JpegBuffer *imageBuffer = NULL;
    size_t base64Len, imageDataLen;

    /* データ URL 形式になっているため、接頭辞部分を取り除く */
    base64Len = base64WithDataURLPrefixRemoved(srcStr, &base64Str);
    if(base64Len == 0) goto FINALIZE;

    /* Base64 化された画像をバイト配列に変換する */
    imageDataLen = base64Decode(base64Str, base64Len, &imageData);
    if(imageDataLen == 0) goto FINALIZE;

    /* バイト配列を JpegBuffer にコピーする */
    copyByteArrayToBinaryData((unsigned char *)imageData, imageDataLen, &imageBuffer);

FINALIZE:
    /* メモリ解放 */
    _SECURE_FREE(imageData);
    _SECURE_FREE(base64Str);

    return imageBuffer;
}

/*! @} */
