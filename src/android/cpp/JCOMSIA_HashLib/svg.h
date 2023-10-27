/*!
@file svg.h
@date Created on: 2020/03/03
@author DATT JAPAN Inc.
@version 3.1
@brief SVG ファイルパース処理
*/

#ifndef svg_h
#define svg_h

#include "common.h"

/*!
@enum SVGResult
@brief SVG 関連処理の結果を示す。
*/
typedef enum _svg_result
{
    SVG_SUCCESS,                                        /*!< @brief 正常終了 */

    SVG_FAILURE_INCORRECT_PARAMETER,                    /*!< @brief 不正な引数を受け取った */
    SVG_FAILURE_FILE_CAN_NOT_OPEN,                      /*!< @brief SVG ファイルを開くことができなかった */
    SVG_FAILURE_FILE_READ_ERROR,                        /*!< @brief SVG ファイルの読み込み時にエラーが発生した */
    SVG_FAILURE_FILE_CLOSE_FAILED,                      /*!< @brief SVG ファイルのクローズ時にエラーが発生した */
    SVG_FAILURE_PARSE_ERROR,                            /*!< @brief SVG ファイルの解析に関して XML パーサからエラーが返された */
    SVG_FAILURE_BROKEN_STRUCTURE,                       /*!< @brief SVG の構造が壊れている */

    SVG_FAILURE_METADATA_BROKEN_STRUCTURE,              /*!< @brief メタデータの構成が壊れている */
    SVG_FAILURE_METADATA_INCORRECT_VENDER,              /*!< @brief 「ベンダー名」が正しくない */
    SVG_FAILURE_METADATA_INCORRECT_SOFTWARE,            /*!< @brief 「ソフトウェア名」が正しくない */
    SVG_FAILURE_METADATA_INCORRECT_META_VERSION,        /*!< @brief 「メタデータバージョン」が正しくない */
    SVG_FAILURE_METADATA_INCORRECT_STANDARD_VERSION,    /*!< @brief 「適用基準バージョン」が正しくない */
    SVG_FAILURE_METADATA_INCORRECT_HASHCODE,            /*!< @brief 「ハッシュコード」が正しくない */

    SVG_FAILURE_INCORRECT_IMAGE_DETECTED,               /*!< @brief 正しくない画像を検出した */

    SVG_FAILURE_ORIGINAL_IMAGE_DOES_NOT_EXIST,          /*!< @brief 必須要素の原本画像を取得できなかった */
    SVG_FAILURE_COULD_NOT_READ_ORIGINAL_IMAGE,          /*!< @brief 原本画像の読み取りに失敗した */

    SVG_FAILURE_CHALKBOARD_IMAGE_DOES_NOT_EXIST,        /*!< @brief 必須となる条件のうえで黒板画像を取得できなかった */
    SVG_FAILURE_COULD_NOT_READ_CHALKBOARD_IMAGE,        /*!< @brief 黒板画像の読み取りに失敗した */

    SVG_FAILURE_OTHER_ERROR,                            /*!< @brief メモリ確保に失敗したなど、その他のエラー */
} SVGResult;

/*!
@brief SVG ファイルを解析して各種データを取得する。
@details
オリジナル画像は必ず存在しなければならない。
黒板画像とハッシュ値は場合によっては存在しないことがある。

@param [in] filePath SVG 画像のファイルパス
@param [out] imageBuffer オリジナル画像領域の画像データ
@param [out] chalkboardBuffer 黒板画像領域の画像データ
@param [out] hashBuffer メタデータ領域のハッシュ値

@retval SVG_SUCCESS 正常終了
@retval SVG_FILE_OPEN_FAILED SVG ファイルを開くことができなかった
@retval SVG_FILE_READ_ERROR SVG ファイルの読み込み時にエラーが発生した
@retval SVG_FILE_CLOSE_FAILED SVG ファイルのクローズ時にエラーが発生した
@retval SVG_CAN_NOT_CREATE_PARSER Expat の XML パーサ準備に失敗した
@retval SVG_ORIGINAL_IMAGE_DOES_NOT_EXIST オリジナル画像を取得できなかった
@retval SVG_OTHER_ERROR メモリ確保に失敗したなど、その他のエラー
*/
SVGResult parse(const char *filePath, JpegBuffer **imageBuffer, JpegBuffer **chalkboardBuffer, HashBuffer **hashBuffer);

#endif /* svg_h */
