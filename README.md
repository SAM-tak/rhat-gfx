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
| `samples/mini_doom.rh` | Goose の `31_mini_doom.goose` の移植。Freedoom E1M1 の WAD を読み、壁・床・スプライト・HUD・音まで動く小さな Doom 風ゲーム |
| `samples/mini_minecraft.rh` | Goose の `30_mini_minecraft.goose` の移植。地形・洞窟・木・テクスチャ・空・フォントまでコードで作る小さなブロックの世界。採掘・クラフト・設置・セーブ |
| `scripts/run.ps1` | ネイティブ層をビルドし、R^ のプログラムをリンクして実行する（Windows） |

## 試す

```powershell
git submodule update --init
. ..\rhat\scripts\devshell.ps1                  # clang を PATH に
.\scripts\run.ps1 test\triangle.rh -Build       # triangle.png をカレントディレクトリに保存
```

`rhat` は `../rhat/build/Release/rhat.exe` を使います（`-Rhat` で変更）。

mini doom は WAD のパスを渡して実行します（WAD は Goose の `samples/data/freedoom1-e1m1.wad`）。

```powershell
.\scripts\run.ps1 samples\mini_doom.rh -Arguments '--wad','..\goose\samples\data\freedoom1-e1m1.wad'
```

WASD で移動、Shift で走る、マウスか左右キーで旋回、E/Space で使用、クリックか Ctrl で射撃、1〜4 で武器、
Tab でマップ、H でヘルプ、Escape で一時停止、F12 でスクリーンショット。`--frames N` で N フレーム描いて
`mini_doom.png` を保存して終わり、`--walk` は W を押し続けます（試験用）。

mini minecraft は引数なしで動きます。`--test` は窓を開かずにゲームの検査だけを行い、`--frames N` は N フレーム描いて
`mini_minecraft.png` を保存して終わり、`--seed N` で地形が変わります。

```powershell
.\scripts\run.ps1 samples\mini_minecraft.rh -Arguments '--test'
```

WASD で移動、Space でジャンプ・泳ぐ、Shift で走る、Ctrl で忍び足、マウスか矢印キーで視点、左クリック長押しで採掘、
右クリックか E で設置、1〜8 かホイールで選択、中クリックで狙ったものを選択、C でクラフト、H でヘルプ、
F5 でセーブ、F9 でロード、F12 でスクリーンショット。

## 状況

- 動作確認済み（Windows）: ウィンドウとフレームループ、GLSL のコンパイル、頂点バッファ（`a.bytes`）、
  インデックスバッファ（`bind_index` と `draw_indexed`、u32^ の添字）、頂点フォーマットの指定（`pipeline` の formats）、テクスチャとサンプラー、
  ユニフォーム、マウスホイール、キーボード・マウス・フォーカスの入力、音の再生、スクリーンショット。
- 入力は、フォーカスのないウィンドウでは捨てられます（goose_gfx の仕様）。隠したウィンドウで試すときは `inject_focus(true^)` を先に送ります。
- Linux・macOS のビルドとスクリプトは未整備です。

## ライセンス

Apache License 2.0（goose_gfx と同じ）。SDL と cute_spirv はそれぞれのライセンスに従います。
