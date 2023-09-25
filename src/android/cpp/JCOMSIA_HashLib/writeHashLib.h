/*!
@file writeHashLib.h
@date Created on: 2015/08/10
@author DATT JAPAN Inc.
@version 3.1
@brief 改ざんチェック用ハッシュ埋め込みモジュールのヘッダ
*/

#ifndef WRITEHASHLIB_H_
#define WRITEHASHLIB_H_

#include <stddef.h>

/*!
@def __has_attribute
@brief 引数の属性が実装されている場合に 1 を返す。
@details このマクロ自体が定義されていない場合は常に 0 を返す。
*/
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

/*!
@def __has_extension
@brief 機能が実装されている場合に 1 を返す。
@details このマクロ自体が定義されていない場合は __has_feature マクロを呼び出す。
*/
#ifndef __has_extension
#define __has_extension __has_feature
#endif

/*!
@def __has_feature
@brief 機能が実装されている場合に 1 を返す。
@details このマクロ自体が定義されていない場合は常に 0 を返す。
*/
#ifndef __has_feature
#define __has_feature(x) 0
#endif

/*!
@def DECLSPEC_PORT
@brief コンパイル条件により DLL インポート / エクスポートを自動振り分けする定義
*/
#ifdef _MSC_VER
#include <windows.h>
#ifdef JACIC_HASH_EXPORTS
#define DECLSPEC_PORT __declspec( dllexport )
#else
#define DECLSPEC_PORT __declspec( dllimport )
#endif
#else

/*!
@def WINAPI
@brief Visual C++ 以外で利用している場合に Windows アプリケーションを無効にするための定義
*/
#define WINAPI
#define DECLSPEC_PORT
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
@def _JCOMSIA_DEPRECATED
@brief 一部の環境に対して非推奨であることを伝える属性を付与する。
*/
#if defined(_MSC_VER)

    /* Visual Studio */
    #define _JCOMSIA_DEPRECATED(message, replacement) __declspec( deprecated(message) )

#elif defined(__APPLE__) || defined(__ANDROID__)

    /* Xcode または Android Studio */
    #define _JCOMSIA_DEPRECATED(message, replacement) __attribute__(( deprecated(message, replacement) ))

#elif __has_extension(attribute_deprecated_with_message) || (defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 405))

    /* GCC 4.5 以上 またはコメント付き deprecated 属性をサポートしている */
    #define _JCOMSIA_DEPRECATED(message, replacement) __attribute__(( deprecated(message) ))

#elif __has_attribute(deprecated) || (defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 301))

    /* GCC 3.1 以上 または deprecated 属性を最低限サポートしている */
    #define _JCOMSIA_DEPRECATED(message, replacement) __attribute__(( deprecated ))

#else

    /* それ以外の環境の場合はこの定義を無視する */
    #define _JCOMSIA_DEPRECATED(message, replacement)

#endif /* _JCOMSIA_DEPRECATED */

/*!
@brief ハッシュ値の長さ
@since 3.1
*/
extern const size_t JCOMSIA_HASH_LENGTH;

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

@since 3.1
*/
#ifdef WRITE_EXPORTS
DECLSPEC_PORT
#endif /* WRITE_EXPORTS */
int WINAPI JCOMSIA_WriteHashValue(const char *sourceFile, const char *destFile);

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

@since 3.1
*/
#ifdef CHECK_EXPORTS
DECLSPEC_PORT
#endif /* CHECK_EXPORTS */
int WINAPI JCOMSIA_CheckHashValue(const char *checkFile);

/*!
@brief 原本画像と黒板画像から SVG 画像に埋め込むための改ざん検知情報を計算する。
@details hashCode は処理が成功した際にメモリ領域が確保されるため、必要に応じて手動で解放する必要がある。
hashCode の長さは `JCOMSIA_HASH_LENGTH` を参照。

@attention 引数 `hashcode` に渡したポインタへハッシュ値が格納された場合、確保したメモリを使用後に `JCOMSIA_SVG_FreeHashValue()` 関数で解放する必要がある。

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

@since 3.1
*/
#ifdef WRITE_EXPORTS
DECLSPEC_PORT
#endif /* WRITE_EXPORTS */
int WINAPI JCOMSIA_SVG_CalculateHashValue(const char *originalImagePath, const char *chalkboardPath, unsigned char **hashCode);

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

@since 3.1
*/
#ifdef CHECK_EXPORTS
DECLSPEC_PORT
#endif /* CHECK_EXPORTS */
int WINAPI JCOMSIA_SVG_CheckHashValue(const char *checkFilePath);

/*!
@brief `JCOMSIA_SVG_CalculateHashValue()` 関数によって確保されたメモリ領域を解放する。
@details 解放されたポインタには NULL が設定される。
@attention 動作の安全性のため、本ライブラリによって確保されたメモリ領域以外を指すポインタを引数として与えないこと。

@param [out] ptr メモリ解放の対象となるポインタの参照。処理後は `NULL` ポインタを指すように設定される。

@since 3.1
*/
#ifdef WRITE_EXPORTS
DECLSPEC_PORT
#endif /* WRITE_EXPORTS */
void WINAPI JCOMSIA_SVG_FreeHashValue(unsigned char **ptr);


/*!
@name deprecated
@{
*/

/*!
@def JACIC_HASH_LENGTH
@deprecated 代わりに `JCOMSIA_HASH_LENGTH` を使用してください。

@brief ハッシュ値の長さ
@since 3.0
*/
#define JACIC_HASH_LENGTH JCOMSIA_HASH_LENGTH

/*!
@deprecated 代わりに `JCOMSIA_WriteHashValue()` を使用してください。

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
#ifdef WRITE_EXPORTS
DECLSPEC_PORT
#endif /* WRITE_EXPORTS */
_JCOMSIA_DEPRECATED("代わりに `JCOMSIA_WriteHashValue()` を使用してください。", "JCOMSIA_WriteHashValue")
int WINAPI JACIC_WriteHashValue(const char *sourceFile, const char *destFile);

/*!
@deprecated 代わりに `JCOMSIA_CheckHashValue()` を使用してください。

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
#ifdef CHECK_EXPORTS
DECLSPEC_PORT
#endif /* CHECK_EXPORTS */
_JCOMSIA_DEPRECATED("代わりに `JCOMSIA_CheckHashValue()` を使用してください。", "JCOMSIA_CheckHashValue")
int WINAPI JACIC_CheckHashValue(const char *checkFile);

/*!
@deprecated 代わりに `JCOMSIA_SVG_CalculateHashValue()` を使用してください。

@brief 原本画像と黒板画像から SVG 画像に埋め込むための改ざん検知情報を計算する。
@details hashCode は処理が成功した際にメモリ領域が確保されるため、必要に応じて手動で解放する必要がある。
hashCode の長さは `JACIC_HASH_LENGTH` を参照。

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
#ifdef WRITE_EXPORTS
DECLSPEC_PORT
#endif /* WRITE_EXPORTS */
_JCOMSIA_DEPRECATED("代わりに `JCOMSIA_SVG_CalculateHashValue()` を使用してください。", "JCOMSIA_SVG_CalculateHashValue")
int WINAPI JACIC_SVG_CalculateHashValue(const char *originalImagePath, const char *chalkboardPath, unsigned char **hashCode);

/*!
@deprecated 代わりに `JCOMSIA_SVG_CheckHashValue()` を使用してください。

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
#ifdef CHECK_EXPORTS
DECLSPEC_PORT
#endif /* CHECK_EXPORTS */
_JCOMSIA_DEPRECATED("代わりに `JCOMSIA_SVG_CheckHashValue()` を使用してください。", "JCOMSIA_SVG_CheckHashValue")
int WINAPI JACIC_SVG_CheckHashValue(const char *checkFilePath);

/*!
@deprecated 代わりに `JCOMSIA_SVG_FreeHashValue()` を使用してください。

@brief `JACIC_SVG_CalculateHashValue()` 関数によって確保されたメモリ領域を解放する。
@details 解放されたポインタには NULL が設定される。
@attention 動作の安全性のため、本ライブラリによって確保されたメモリ領域以外を指すポインタを引数として与えないこと。

@param [out] ptr メモリ解放の対象となるポインタの参照。処理後は `NULL` ポインタを指すように設定される。

@since 3.1
*/
#ifdef WRITE_EXPORTS
DECLSPEC_PORT
#endif /* WRITE_EXPORTS */
_JCOMSIA_DEPRECATED("代わりに `JCOMSIA_SVG_FreeHashValue()` を使用してください。", "JCOMSIA_SVG_FreeHashValue")
void WINAPI JACIC_SVG_FreeHashValue(unsigned char **ptr);

/* @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* WRITEHASHLIB_H_ */
