# rhat-gfx

R^ から使うウィンドウ・入力・GPU 描画・音のライブラリです。
[goose_gfx](https://github.com/aardappel/goose_gfx) の C 層（SDL3 の GPU API 上の `gfx`、SDL3 の `audio`）と、
Goose の `embed_shader` が使うシェーダコンパイラ（cute_spirv）を流用し、R^ の `externC^` から呼べる形の
薄い C 中継（`src/shim`）と R^ のモジュール（`gfx.rh`）を付けています。

## 構成

| | |
|---|---|
| `gfx.rh` | R^ のモジュール `gfx`。ウィンドウ、入力、バッファ、テクスチャ、パイプライン、描画 |
| `src/shim/` | C 中継。goose_gfx の API は `{data, len}` のスライスやパック構造体を値で渡すので、スカラーと (ポインタ, 長さ) だけの関数に包む。シェーダの GLSL はここで実行時にコンパイルする |
| `src/gfx/`, `src/audio/` | goose_gfx の C 層（そのまま） |
| `src/shaderc/`, `third_party/cute_spirv/` | GLSL 450 を SPIR-V・MSL・HLSL へ。Goose のコンパイラのもの |
| `third_party/SDL` | SDL3（サブモジュール、shallow） |
| `test/` | `clear.rh`（画面クリア）、`triangle.rh`（シェーダと描画） |
| `scripts/run.ps1` | ネイティブ層をビルドし、R^ のプログラムをリンクして実行する（Windows） |

## 試す

```powershell
git submodule update --init
. ..\rhat\scripts\devshell.ps1                  # clang を PATH に
.\scripts\run.ps1 test\triangle.rh -Build       # build\triangle.png も保存される
```

`rhat` は `../rhat/build/Release/rhat.exe` を使います（`-Rhat` で変更）。

## 状況

- 動作確認済み: ウィンドウ、クリア、GLSL のコンパイルと三角形、スクリーンショット（Windows、Vulkan/D3D12）。
- 入力・音・バッファ・テクスチャは中継に入っていますが、R^ 側のテストはまだありません。
- 頂点バッファに R^ の配列を渡すには、`f32^[]` などを `u8^[:]` として見る変換が R^ に必要です（未対応）。
- Linux・macOS のビルドとスクリプトは未整備です。

## ライセンス

Apache License 2.0（goose_gfx と同じ）。SDL と cute_spirv はそれぞれのライセンスに従います。
