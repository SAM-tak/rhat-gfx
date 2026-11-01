# rhat-gfx

R^ から使うウィンドウ・入力・GPU 描画・音のライブラリです。
[goose_gfx](https://github.com/aardappel/goose_gfx) の C 層（SDL3 の GPU API 上の `gfx`、SDL3 の `audio`）と、
Goose の `embed_shader` が使うシェーダコンパイラ（cute_spirv）を流用し、R^ の `externC^` から呼べる形の
薄い C 中継（`src/shim`）と R^ のモジュール（`gfx.rh`）を付けています。

## 構成

| | |
|---|---|
| `gfx.rh` | R^ のモジュール `gfx`。ウィンドウ、入力（テスト用の注入を含む）、バッファ、テクスチャ、パイプライン、描画 |
| `audio.rh` | R^ のモジュール `audio`。f32 の PCM を音として登録し、音量・パン・ピッチを付けて再生 |
| `src/shim/` | C 中継。goose_gfx の API は `{data, len}` のスライスやパック構造体を値で渡すので、スカラーと (ポインタ, 長さ) だけの関数に包む。シェーダの GLSL はここで実行時にコンパイルする |
| `src/gfx/`, `src/audio/` | goose_gfx の C 層（そのまま） |
| `src/shaderc/`, `third_party/cute_spirv/` | GLSL 450 を SPIR-V・MSL・HLSL へ。Goose のコンパイラのもの |
| `third_party/SDL` | SDL3（サブモジュール、shallow） |
| `test/` | `clear.rh`（画面クリア）、`triangle.rh`（シェーダと描画）、`buffer.rh`（頂点バッファ）、`texture.rh`（テクスチャ・サンプラー・ユニフォーム）、`input.rh`（注入した入力の検査）、`audio.rh`（音の再生開始） |
| `scripts/run.ps1` | ネイティブ層をビルドし、R^ のプログラムをリンクして実行する（Windows） |

## 試す

```powershell
git submodule update --init
. ..\rhat\scripts\devshell.ps1                  # clang を PATH に
.\scripts\run.ps1 test\triangle.rh -Build       # build\triangle.png も保存される
```

`rhat` は `../rhat/build/Release/rhat.exe` を使います（`-Rhat` で変更）。

## 状況

- 動作確認済み（Windows）: ウィンドウとフレームループ、GLSL のコンパイル、頂点バッファ（`a.bytes`）、テクスチャとサンプラー、
  ユニフォーム、キーボード・マウス・フォーカスの入力、音の再生、スクリーンショット。
- 入力は、フォーカスのないウィンドウでは捨てられます（goose_gfx の仕様）。隠したウィンドウで試すときは `inject_focus(true^)` を先に送ります。
- Linux・macOS のビルドとスクリプトは未整備です。

## ライセンス

Apache License 2.0（goose_gfx と同じ）。SDL と cute_spirv はそれぞれのライセンスに従います。
