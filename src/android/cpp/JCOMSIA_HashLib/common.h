/*!
@file common.h
@date Created on: 2015/08/18
@author DATT JAPAN Inc.
@version 3.1
@brief 共通処理用ヘッダ
*/

#ifndef COMMON_H_
#define COMMON_H_

#include <stddef.h>

/*!
@name フラグ定義
@{
*/
#ifndef JACIC_FLAGS_
#define JACIC_FLAGS_

/*!
@enum JACIC_BOOL
@brief 真偽値
*/
typedef enum
{
    JACIC_BOOL_FALSE    = 0, /*!< @brief 偽 */
    JACIC_BOOL_TRUE     = 1  /*!< @brief 真 */
} JACIC_BOOL;

#endif /* JACIC_FLAGS_ */
/*! @} */

/*!
@name エンディアンフラグ定義
@{
*/
#ifndef JACIC_ENDIANS_
#define JACIC_ENDIANS_

/*!
@enum JACIC_ENDIANS
@brief エンディアンフラグ
*/
typedef enum
{
    JACIC_BIG_ENDIAN    = 0, /*!< @brief ビッグエンディアン */
    JACIC_LITTLE_ENDIAN = 1  /*!< @brief リトルエンディアン */
} JACIC_ENDIANS;

#endif /* JACIC_ENDIANS_ */
/*! @} */

/*!
@name 関数のリターンコード定義

@details  0          : 正常終了
@details  1          : ハッシュ値が正常（チェック時のみ）
@details  0, -1, -2  : ハッシュ値の不正（チェック時のみ）
@details -101 ～     : 引数の不正などコード上の問題
@details -201 ～     : ファイル操作関連全般の問題
@details -301 ～     : Exif ファイル操作関連の問題
@details -401 ～     : SVG ファイル操作関連の問題
@details -900        : その他の問題

@{
*/
#define FUNCTION_SUCCESS                      ((int)   0)                /*!< @brief 関数が正常に終了した */

#define SAME_HASH                             ((int)   0)                /*!< @brief ハッシュ値が正しい */
#define INCORRECT_HASH_IMAGE                  ((int)  -1)                /*!< @brief ハッシュ値（画像）が正しくない */
#define INCORRECT_HASH_DATE                   ((int)  -2)                /*!< @brief ハッシュ値（撮影日時）が正しくない */
#define INCORRECT_HASH_BOTH                   ((int)  -3)                /*!< @brief ハッシュ値（画像、撮影日時）が正しくない */

#define INCORRECT_PARAMETER                   ((int)-101)                /*!< @brief 不正な引数が指定された場合 */
#define SAME_FILE_PATH                        ((int)-102)                /*!< @brief 読込元と出力先の画像ファイルパスが同じ */

#define FILE_NOT_EXISTS                       ((int)-201)                /*!< @brief 読込元画像ファイルが存在しない */
#define FILE_ALREADY_EXISTS                   ((int)-202)                /*!< @brief 出力先画像ファイルが既に存在する */
#define FILE_OPEN_FAILED                      ((int)-203)                /*!< @brief ファイルオープン失敗 */
#define FILE_SIZE_ZERO                        ((int)-204)                /*!< @brief 読み込んだファイルサイズがゼロ */
#define FILE_WRITE_FAILED                     ((int)-205)                /*!< @brief ファイル書き込み失敗 */
#define FILE_CLOSE_FAILED                     ((int)-206)                /*!< @brief ファイルのクローズに失敗 */

#define INCORRECT_EXIF_FORMAT                 ((int)-301)                /*!< @brief Exif フォーマットが不正 */
#define APP5_ALREADY_EXISTS                   ((int)-302)                /*!< @brief APP5 セグメントが既に存在する */
#define APP5_NOT_EXISTS                       ((int)-303)                /*!< @brief APP5 領域が見つからない */
#define INCORRECT_APP5_FORMAT                 ((int)-304)                /*!< @brief APP5 領域の記述形式が異なる */
#define HASH_NOT_EXISTS                       ((int)-305)                /*!< @brief ハッシュ値が見つからない */
#define INCORRECT_APP1_FORMAT                 ((int)-306)                /*!< @brief APP1 領域の記述形式が異なる */
#define DATE_NOT_EXISTS                       ((int)-307)                /*!< @brief 日時情報が見つからない */
#define INCORRECT_TEXT                        ((int)-308)                /*!< @brief テキスト情報が不正 */
#define INCORRECT_IDENTIFIER                  ((int)-309)                /*!< @brief 識別子が不正 */
#define APP1_NOT_EXISTS                       ((int)-310)                /*!< @brief APP1 領域が見つからない */

#define ORG_DOES_NOT_HAVE_HASH                ((int)-351)                /*!< @brief 原本画像に画像ハッシュまたは撮影日時ハッシュが含まれていない */
#define ORG_HASH_NG_IMAGE                     ((int)-352)                /*!< @brief 原本画像のハッシュ値（画像）が正しくない */
#define ORG_HASH_NG_DATE                      ((int)-353)                /*!< @brief 原本画像のハッシュ値（撮影日時）が正しくない */
#define ORG_HASH_NG_BOTH                      ((int)-354)                /*!< @brief 原本画像のハッシュ値（画像、撮影日時）が正しくない */

#define CB_DOES_NOT_HAVE_HASH                 ((int)-361)                /*!< @brief 黒板画像に画像ハッシュまたは撮影日時ハッシュが含まれていない */
#define CB_HASH_NG_IMAGE                      ((int)-362)                /*!< @brief 黒板画像のハッシュ値（画像）が正しくない */
#define CB_HASH_NG_DATE                       ((int)-363)                /*!< @brief 黒板画像のハッシュ値（撮影日時）が正しくない */
#define CB_HASH_NG_BOTH                       ((int)-364)                /*!< @brief 黒板画像のハッシュ値（画像、撮影日時）が正しくない */

#define COMBINATION_HASHES_DOES_NOT_MATCH     ((int)-400)                /*!< @brief 原本画像と黒板画像の組み合わせのハッシュ値が一致しない */
#define ORG_DOES_NOT_EXIST                    ((int)-401)                /*!< @brief 原本画像が存在しない */

#define OTHER_ERROR                           ((int)-900)                /*!< @brief その他のエラー */
/*! @} */

/*!
@name ハッシュ値埋め込み処理リターンコード

@details    0    : 正常終了
@details -101 ～ : 引数の不正などコード上の問題
@details -201 ～ : ファイル操作関連全般の問題
@details -301 ～ : Exif ファイル操作関連の問題
@details -900    : その他の問題

@{
*/
#define JW_SUCCESS                            FUNCTION_SUCCESS         /*!< @brief 正常終了 */

#define JW_ERROR_INCORRECT_PARAMETER          INCORRECT_PARAMETER      /*!< @brief 不正な引数が指定された場合 */
#define JW_ERROR_SAME_FILE_PATH               SAME_FILE_PATH           /*!< @brief 読込元と出力先の画像ファイルパスが同じ */

#define JW_ERROR_READ_FILE_NOT_EXISTS         FILE_NOT_EXISTS          /*!< @brief 読込元画像ファイルが存在しない */
#define JW_ERROR_WRITE_FILE_ALREADY_EXISTS    FILE_ALREADY_EXISTS      /*!< @brief 出力先画像ファイルが既に存在する */
#define JW_ERROR_READ_FILE_OPEN_FAILED        FILE_OPEN_FAILED         /*!< @brief ファイルオープン失敗 */
#define JW_ERROR_READ_FILE_SIZE_ZERO          FILE_SIZE_ZERO           /*!< @brief 読み込んだファイルサイズがゼロ */
#define JW_ERROR_WRITE_FILE_FAILED            FILE_WRITE_FAILED        /*!< @brief ファイル書き込み失敗 */
#define JW_ERROR_FILE_CLOSE_FAILED            FILE_CLOSE_FAILED        /*!< @brief ファイルのクローズに失敗 */

#define JW_ERROR_INCORRECT_EXIF_FORMAT        INCORRECT_EXIF_FORMAT    /*!< @brief Exif フォーマットが不正 */
#define JW_ERROR_APP5_ALREADY_EXISTS          APP5_ALREADY_EXISTS      /*!< @brief APP5 セグメントが既に存在する */
#define JW_ERROR_DATE_NOT_EXISTS              DATE_NOT_EXISTS          /*!< @brief 読込元画像に日時情報が見つからない */

#define JW_ERROR_OTHER                        OTHER_ERROR              /*!< @brief その他のエラー */
/*! @} */

/*!
@name ハッシュ値チェック処理リターンコード

@details 1         : 正常終了、かつ正しい
@details 0, -1, -2 : 正常終了、かつ正しくない
@details -101 ～   : 引数の不正などコード上の問題
@details -201 ～   : ファイル操作関連全般の問題
@details -301 ～   : Exif ファイル操作関連の問題
@details -900      : その他の問題

@{
*/
#define JC_OK                               ((int) 1)                   /*!< @brief ハッシュ値が正しい */
#define JC_NG                               ((int) 0)                   /*!< @brief ハッシュ値（画像、撮影日時）が正しくない */
#define JC_NG_IMAGE                         ((int)-1)                   /*!< @brief ハッシュ値（画像）が正しくない */
#define JC_NG_DATE                          ((int)-2)                   /*!< @brief ハッシュ値（撮影日時）が正しくない */

#define JC_ERROR_INCORRECT_PARAMETER        INCORRECT_PARAMETER         /*!< @brief 不正な引数が指定された場合 */

#define JC_ERROR_READ_FILE_NOT_EXISTS       FILE_NOT_EXISTS             /*!< @brief 読込元画像ファイルが存在しない */
#define JC_ERROR_READ_FILE_OPEN_FAILED      FILE_OPEN_FAILED            /*!< @brief ファイルオープン失敗 */
#define JC_ERROR_READ_FILE_SIZE_ZERO        FILE_SIZE_ZERO              /*!< @brief 読み込んだファイルサイズがゼロ */
#define JC_ERROR_FILE_CLOSE_FAILED          FILE_CLOSE_FAILED           /*!< @brief ファイルのクローズに失敗 */

#define JC_ERROR_INCORRECT_EXIF_FORMAT      INCORRECT_EXIF_FORMAT       /*!< @brief Exif ファイルフォーマットが不正 */
#define JC_ERROR_APP5_NOT_EXISTS            APP5_NOT_EXISTS             /*!< @brief APP5 領域が見つからない */
#define JC_ERROR_INCORRECT_APP5_FORMAT      INCORRECT_APP5_FORMAT       /*!< @brief APP5 領域の記述形式が異なる */
#define JC_ERROR_HASH_NOT_EXISTS            HASH_NOT_EXISTS             /*!< @brief ハッシュ値が設定されていない */
#define JC_ERROR_DATE_NOT_EXISTS            DATE_NOT_EXISTS             /*!< @brief 読込元画像に日時情報が見つからない */

#define JC_ERROR_OTHER                      OTHER_ERROR                 /*!< @brief その他のエラー */
/*! @} */

/*!
@name SVG ファイル埋め込み用ハッシュ値計算処理リターンコード

@details    0    : 正常終了
@details -101 ～ : 引数の不正などコード上の問題
@details -201 ～ : ファイル操作関連全般の問題
@details -301 ～ : Exif ファイル操作関連の問題
@details -900    : その他の問題

@{
*/

#define JW_HASHER_CREATE_SUCCESS                FUNCTION_SUCCESS        /*!< @brief 正常終了 */
#define JW_HASHER_ERROR_INCORRECT_PARAMETER     INCORRECT_PARAMETER     /*!< @brief 引数に NULL を渡された等、引数指定に不備がある場合 */

#define JW_HASHER_ERROR_FILE_DOES_NOT_EXIST     FILE_NOT_EXISTS         /*!< @brief 読み込み元ファイルが存在しない */
#define JW_HASHER_ERROR_FILE_OPEN_FAILED        FILE_OPEN_FAILED        /*!< @brief ファイルオープン失敗 */
#define JW_HASHER_ERROR_FILE_SIZE_ZERO          FILE_SIZE_ZERO          /*!< @brief 読み込んだファイルサイズがゼロ */
#define JW_HASHER_ERROR_FILE_CLOSE_FAILED       FILE_CLOSE_FAILED       /*!< @brief ファイルクローズ失敗 */

#define JW_HASHER_ERROR_ORG_DOES_NOT_HAVE_HASH  ORG_DOES_NOT_HAVE_HASH  /*!< @brief 原本画像に画像ハッシュまたは撮影日時ハッシュが含まれていない */
#define JW_HASHER_ERROR_ORG_HASH_NG_IMAGE       ORG_HASH_NG_IMAGE       /*!< @brief 原本画像のハッシュ値（画像）が正しくない */
#define JW_HASHER_ERROR_ORG_HASH_NG_DATE        ORG_HASH_NG_DATE        /*!< @brief 原本画像のハッシュ値（撮影日時）が正しくない */
#define JW_HASHER_ERROR_ORG_HASH_NG_BOTH        ORG_HASH_NG_BOTH        /*!< @brief 原本画像のハッシュ値（画像、撮影日時）が正しくない */

#define JW_HASHER_ERROR_CB_DOES_NOT_HAVE_HASH   CB_DOES_NOT_HAVE_HASH   /*!< @brief 黒板画像に画像ハッシュまたは撮影日時ハッシュが含まれていない */
#define JW_HASHER_ERROR_CB_HASH_NG_IMAGE        CB_HASH_NG_IMAGE        /*!< @brief 黒板画像のハッシュ値（画像）が正しくない */
#define JW_HASHER_ERROR_CB_HASH_NG_DATE         CB_HASH_NG_DATE         /*!< @brief 黒板画像のハッシュ値（撮影日時）が正しくない */
#define JW_HASHER_ERROR_CB_HASH_NG_BOTH         CB_HASH_NG_BOTH         /*!< @brief 黒板画像のハッシュ値（画像、撮影日時）が正しくない */

#define JW_HASHER_ERROR_OTHERS                  OTHER_ERROR             /*!< @brief その他のエラー（メモリ領域の確保失敗など） */
/*! @} */

/*!
@name SVG ファイルチェック処理リターンコード

@details    2    : 正常終了、かつ正しい
@details   -3    : 正常終了、かつ正しくない
@details  -21 ～ : 原本画像の問題
@details  -41 ～ : 黒板画像の問題
@details -101    : 引数の不正などコード上の問題
@details -900    : その他の問題

@{
*/
#define JC_SVG_RESULT_OK                                    JC_OK       /*!< @brief 改ざん無し */
#define JC_SVG_RESULT_NG_COMBINATION                        ((int)-3)   /*!< @brief 原本画像と黒板画像の組み合わせが正しくない */

#define JC_SVG_ERROR_ORG_DOES_NOT_EXIST                     ((int)-21)  /*!< @brief SVG 内に原本画像が存在しない */
#define JC_SVG_ERROR_ORG_DOES_NOT_HAVE_HASH                 ((int)-22)  /*!< @brief 原本画像からハッシュ値を取得できなかった */
#define JC_SVG_ERROR_NG_ORG_IMAGE                           ((int)-23)  /*!< @brief 原本画像の画像ハッシュが不正または計算不能 */
#define JC_SVG_ERROR_NG_ORG_DATE                            ((int)-24)  /*!< @brief 原本画像の撮影日時ハッシュが不正または計算不能 */
#define JC_SVG_ERROR_NG_ORG_BOTH                            ((int)-25)  /*!< @brief 原本画像の画像ハッシュおよび撮影日時ハッシュの両方が不正または計算不能 */

#define JC_SVG_ERROR_CB_DOES_NOT_EXIST                      ((int)-41)  /*!< @brief SVG 内に合成ハッシュ値が含まれているが、黒板画像が存在しない */
#define JC_SVG_ERROR_CB_DOES_NOT_HAVE_HASH                  ((int)-42)  /*!< @brief 黒板画像からハッシュ値を取得できなかった */
#define JC_SVG_ERROR_NG_CB_IMAGE                            ((int)-43)  /*!< @brief 黒板画像の画像ハッシュが不正または計算不能 */
#define JC_SVG_ERROR_NG_CB_DATE                             ((int)-44)  /*!< @brief 黒板画像の撮影日時ハッシュが不正または計算不能 */
#define JC_SVG_ERROR_NG_CB_BOTH                             ((int)-45)  /*!< @brief 黒板画像の画像ハッシュおよび撮影日時ハッシュの両方が不正または計算不能 */

#define JC_SVG_ERROR_BROKEN_STRUCTURE                       ((int)-61)  /*!< @brief SVG の構成が壊れている */

#define JC_SVG_ERROR_METADATA_BROKEN_STRUCTURE              ((int)-81)  /*!< @brief メタデータの構成が壊れている */
#define JC_SVG_ERROR_METADATA_INCORRECT_VENDER              ((int)-82)  /*!< @brief メタデータ「ベンダー名」に問題あり */
#define JC_SVG_ERROR_METADATA_INCORRECT_SOFTWARE            ((int)-83)  /*!< @brief メタデータ「ソフトウェア名」に問題あり */
#define JC_SVG_ERROR_METADATA_INCORRECT_FORMAT_VERSION      ((int)-84)  /*!< @brief メタデータ「メタデータバージョン」に問題あり */
#define JC_SVG_ERROR_METADATA_INCORRECT_STANDARD_VERSION    ((int)-85)  /*!< @brief メタデータ「適用基準バージョン」に問題あり */
#define JC_SVG_ERROR_METADATA_INCORRECT_HASHCODE            ((int)-86)  /*!< @brief メタデータ「ハッシュコード」に問題あり */

#define JC_SVG_ERROR_INCORRECT_PARAMETER        INCORRECT_PARAMETER     /*!< @brief NULLなど不正な引数が渡された */
#define JC_SVG_ERROR_OTHERS                     OTHER_ERROR             /*!< @brief メモリ確保失敗などの予期せぬエラー */
/*! @} */

/*! @name 定数マクロ定義 */
/*! @{ */
#define BYTE_SIZE_UNSIGNED_CHAR  ((size_t) 1) /*!< @brief unsigned char のバイト数 */
#define BYTE_SIZE_UNSIGNED_SHORT ((size_t) 2) /*!< @brief unsigned short のバイト数 */
#define BYTE_SIZE_UNSIGNED_INT   ((size_t) 4) /*!< @brief unsigned int のバイト数 */
#define BIT_SIZE_1BYTE           ((size_t) 8) /*!< @brief 1 バイトのビット数 */
#define BYTE_SIZE_HASH_LENGTH    ((size_t)64) /*!< @brief ハッシュのバイト数 */
/*! @} */


typedef struct _binaryData HashBuffer; /*!< @brief バイト配列とその長さを持つ構造体 */
typedef struct _binaryData JpegBuffer; /*!< @brief 画像読み込み時に画像のバイト情報とサイズを持つ構造体 */

/*!
@struct _binaryData
@brief 自身の長さと実体を持つバイナリ構造体
*/
struct _binaryData
{
    size_t _len;           /*!< @brief 値の長さ */
    unsigned char _buff[]; /*!< @brief 値 */
};

/*!
@brief 稼働環境のエンディアンを判定し、エンディアンを示す値を返す。
@retval BIG_ENDIAN ビッグエンディアン
@retval LITTLE_ENDIAN リトルエンディアン
*/
JACIC_ENDIANS getEndian(void);

/*!
@brief 符号なし短整数値をエンディアン変換する。
@param src 符号なし短整数値
@return エンディアン変換後の符号なし短整数値
*/
unsigned short swapEndian16(unsigned short src);

/*!
@brief 符号なし整数値をエンディアン変換する。
@param src 符号なし整数値
@return エンディアン変換後の符号なし整数値
*/
unsigned int swapEndian32(unsigned int src);

/*!
@brief 数値を 16 進数の文字列表現で取得する。
@param [out] dst 取得対象の文字変数のアドレス
@param hexChar 文字列表現に変換する 16 進数 1 文字で表現可能な数値
@retval FUNCTION_SUCCESS 正常終了
@retval INCORRECT_PARAMETER hexChar が不正である
*/
int getCharCode(unsigned char *dst, unsigned char hexChar);

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
int writeFile(const char *dst, unsigned char *outBuff, size_t length);

/*!
@brief 実体と長さをもつバイナリデータ構造体のメモリを確保して返す。
@details 確保されたバイナリ領域 _buff は 0x00 で初期化される。
@param dataLen データ部の長さ（binaryData->_len の値に割り当てられる）
@return 確保したバイナリデータ構造体のポインタ。メモリが確保できなかった場合は NULL
*/
struct _binaryData *allocateBinaryData(const size_t dataLen);

/*!
@brief バイト配列構造体の複製を行う。
@details メモリの動的確保が行われるため、dst を使い終わったら解放する必要がある。
@param [in] src コピー元のバイト配列構造体
@param [out] dst コピー先のバイト配列構造体
 */
void copyBinaryDataToBinaryData(const struct _binaryData *src, struct _binaryData **dst);

/*!
@brief バイト配列を構造体に複製する。
@details メモリの動的確保が行われるため、dst を使い終わったら解放する必要がある。
@param [in] srcData コピー元のバイト配列
@param length srcData のデータ長
@param [out] dst コピー先のバイト配列構造体
 */
void copyByteArrayToBinaryData(const unsigned char *srcData, const size_t length, struct _binaryData **dst);

/*!
@def _SECURE_FREE
@brief 渡されたポインタの指すメモリ領域を解放する。
@details 解放されたポインタには NULL が設定される。

@param [in] ptr メモリ解放の対象となるポインタ。処理後は `NULL` ポインタを指す。

@since 3.1
*/
#define _SECURE_FREE(ptr) \
do { \
    if((ptr) != NULL) \
    { \
        free((ptr)); \
        (ptr) = NULL; \
    } \
} \
while(0)

#endif /* COMMON_H_ */
