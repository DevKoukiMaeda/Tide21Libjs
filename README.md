# Tide21Libjs

## 概要
test
ブラウザサイドで潮汐計算を行うモジュールです。
* 潮汐を求めるライブラリであるY.Senta氏によってC++で開発された[Tide21Lib](http://senta.s112.xrea.com/senta/Tips/000/65/index.html)をブラウザ上で動作する形にコンパイルするプロジェクトです。
* モジュール名を変更することで他のwasmライブラリとの衝突を防ぐことを目的にしています。
* 処理結果はJSON形式です。
* DTフォルダ内の調和定数表はすべて**Tide21Lib.data**にアーカイブされます。
**DT/{ファイル名}.TD2**のパスで指定してください。

## 使い方
Releseからダウンロード・解凍後
**読み込むhtmlと同一の階層に配置してください**
```HTML
<script src="Tide21Lib.js"></script>
<script src="tide.js"></script>
```
HTML上で**tide.js**を**script**タグで読み込みます。
### 実行例
```js
var TideData = Tide.calctide(2022,4,8,"DT/14根岸.TD2")
```
変数**TideData**にJSON文字列が格納されます。
```js
{
  sunrise: "05:20",
  sunset: "18:10",
  tidedata: [・・・],
  tidename: "小潮"
}
```
計算結果の要素名は以下です。
**sunrise**=日の出
**sunset**=日没
**tidedata**=潮位
**tidename**=潮回り
### エラー
td2ファイルが見つからない場合
```js
{
  error: "td2 not found"
}
```
td2ファイルのパスが空の場合
```js
{
  error: "td2 path is empty"
}
```
## ビルド

* emsdkをインストール
* 一時的に環境変数にemscriptenを追加するために以下のコマンドを実行
  ```bat
  {emsdkをインストールしたフォルダ}\emsdk_env.bat
  ```
  **実行後コマンドプロンプトを閉じないで次のステップに進んでください**
* 下記コマンドでビルド
  ```
  cd {Tide21Libjsのフォルダ}
  make
  ```
  ビルド完了後コマンドプロンプトは閉じることが出来ます。
* **{Tide21Libjsのフォルダ}\bin**に以下の4ファイルが存在することを確認してください
  * Tide21Lib.data
  * Tide21Lib.js
  * Tide21Lib.wasm
  * tide.js
* この**4ファイル**をWebサイトの**同一のフォルダ**に配置し
  **tide.js**を**script**タグで読み込んでください。



## 説明
### jsとC++との値の受け渡しと関数の実行
```js
var Tide = {}
```
ライブラリをjsから操作するオブジェクトをグローバル変数に定義
```js
    TideMod().then(function(TModule) {・・・});
```
Tide21Libのロードが完了したときのコールバックとして
オブジェクトの初期化を行う関数を渡します。
引数のTModuleがC++ライブラリのオブジェクトです。
以下は初期化関数の重要な箇所の説明です。

### C++の文字列領域をjsから確保する準備
C++で確保した領域はjsからアクセスできないため
ブラウザサイドで領域を確保する必要があります。
```js
      Tide._free = TModule._free;
      Tide.allocate = TModule.allocate;
      Tide.ALLOC_NORMAL = TModule.ALLOC_NORMAL;
      Tide.intArrayFromString = TModule.intArrayFromString;
      Tide.UTF8ToString = TModule.UTF8ToString;
      Tide.MAXBUFFER = 1024*1024;
```
MakeFileで指定したemccのオプションである  
**EXTRA_EXPORTED_RUNTIME_METHODS**で   
jsにエクスポートした関数を利用します。
**Tide.MAXBUFFER**はJSON文字列のバッファーの最大バイト数です。
### C++の関数を実行するオブジェクトの定義
```js
      Tide._calctide = TModule.cwrap(
          'calctide',
          null,
          ['number',//int y
          'number', //int m
          'number',//int d
          'number',//char* td2_path 
          'number'//char* ptrRet
          ]);
```
```cpp
void EMSCRIPTEN_KEEPALIVE calctide(int y,
                                   int m,
                                   int d,
                                   char* td2_path,
                                   char* ptrRet)
```
上段js、下段C
C++の定義に従ってjsからCの関数を呼びだす**cwrap**オブジェクトを定義します。
#### cwrapの引数
1. 関数名
2. 戻り値の型(この場合**void**なので**null**)
3. 引数の型（配列、jsのコメントとしてC++の引数の対応関係を記載）
引数の型の配列の後半2要素は定数表のパスである**char* td2_path**、
JSONの潮汐表の**char* ptrRet**バッファはポインタなので**number**を使用します。
### jsからC++の関数を実行する
以下はjsから指定日時から24時間分の潮位を取得するC++の関数です。
```js
Tide.calctide = function (y, m, d, td2path) {
  var ptr  = Tide.allocate(Tide.intArrayFromString(td2path),  Tide.ALLOC_NORMAL);
  var ptrRet = Tide.allocate(Array(Tide.MAXBUFFER).fill(0),Tide.ALLOC_NORMAL);
  Tide._calctide(y,m,d,ptr,ptrRet);
  var resstr = Tide.UTF8ToString(ptrRet);
  Tide._free(ptr);
  Tide._free(ptrRet);
  return JSON.parse(resstr);
}
```
#### 文字列領域の確保
```js
var ptr  = Tide.allocate(Tide.intArrayFromString(td2path),  Tide.ALLOC_NORMAL);
var ptrRet = Tide.allocate(Array(Tide.MAXBUFFER).fill(0),Tide.ALLOC_NORMAL);
```
**ptr**はtd2pathを**intArrayFromString**整数値の配列をtd2ファイルのパスを   
文字列から生成し、C++のTD2ファイルパスの領域を確保します。
**ptrRet**はJSON文字列の最大バッファーのバイト数分0で初期化した整数値の配列生成し、
C++の潮汐計算結果JSONの領域を確保します。
```js
Tide._calctide(y,m,d,ptr,ptrRet);
var resstr = Tide.UTF8ToString(ptrRet);
```
**Tide._calctide**でC++で定義したcalctideを実行します。
**resstr**は実行結果を文字列に変換します。
```js
Tide._free(ptr);
Tide._free(ptrRet);
return JSON.parse(resstr);
```
**ptr**、**ptrRet**の領域を解放し
JSON文字列を返却します。