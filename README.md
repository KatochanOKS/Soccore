# Soccore Game Engine

DirectX 12 と Win32 で構成された 3D/2D ハイブリッドの自作ゲームエンジンです。`EngineManager` を中心にデバイスやパイプライン、リソースの初期化を行い、シーン（`StartScene`/`GameScene`/`GameOverScene`）を切り替えながらゲームループを進行します。ImGui によるデバッグ UI、Lua によるパラメータホットリロード、FBX スキニングモデル描画など、実装済みの機能をすぐ試せる構成になっています。

## 特長
- **DirectX 12 ベースの描画パイプライン**: デバイス・スワップチェーン・深度バッファを `DeviceManager`/`SwapChainManager`/`DepthBufferManager` で初期化し、`Renderer` がフレームの開始・終了と各オブジェクト描画を担当します。
- **コンポーネント指向の GameObject**: `GameObject` に `Transform`、`StaticMeshRenderer`、`SkinnedMeshRenderer`、`UIImage` などのコンポーネントを付与してシーンを構成。シーン側では `m_SceneObjects` 配列で一括管理します。
- **スキニング & FBX 読み込み**: `FbxModelLoader` と `Animator` により FBX モデルとアニメーションを読み込み、`AnimationManager` がプレイヤー用アニメを登録・再生します。
- **UI と 3D の同時レンダリング**: 3D オブジェクトと 2D UI を別ループで描画し、HP バーやロゴなどの UI を `UIImage` で管理します。
- **シーン管理**: `StartScene` でロゴ表示・Enter キー待ち、`GameScene` で対戦・スロット、`GameOverScene` で終了処理という流れを `EngineManager::ChangeScene` で切替。
- **Lua ホットリロード**: `Player1Component` は `assets/scripts/player1_config.lua` を監視し、変更を検知すると自動で最大 HP、スピード、名前などを再読み込みします。
- **ImGui デバッグ**: マウス座標やレンダリング解像度を確認するデバッグウィンドウを `GameScene` で表示し、DX12/Win32 バックエンドを初期化済みです。

## レンダリングパイプラインの流れ
- **初期化フェーズ**
  - `DeviceManager` がデバイス、コマンドキュー、フェンスを生成し、`SwapChainManager` が 2 バッファのスワップチェーンを構築。
  - `DepthBufferManager` が深度テクスチャを作成し、`PipelineManager` が Root Signature と PSO を設定。`PipelineManager::InitializePipeline` で頂点/ピクセルシェーダー、入力レイアウト、ブレンド/ラスタライザ/深度ステンシルステート、サンプラーをまとめて登録します。
  - `BufferManager` がフレーム数分の定数バッファを 256 バイト境界で確保し、ディスクリプタヒープをセットアップ。テクスチャは `TextureManager` で SRV をまとめ、スキニング用のスケルトン行列バッファもここで更新します。
- **フレームの開始 (`Renderer::BeginFrame`)**
  - 現在のバックバッファを取得し、RTV/DSV をクリア。コマンドアロケータとコマンドリストをリセットし、デフォルトの PSO・ルートシグネチャをバインドします。
- **オブジェクト描画 (`Renderer::Draw` / `Renderer::DrawUI`)**
  - 3D: `StaticMeshRenderer`/`SkinnedMeshRenderer` がそれぞれ VB/IB、ワールド行列、マテリアル定数を設定し、`Renderer` を通じて DrawCall を発行。スキニングは `SkinningVS.hlsl`/`SkinningPS.hlsl` の組み合わせを使用。
  - 2D/UI: `UIImage` がスプライト用の頂点バッファを用意し、`UIPipeline` の PSO で描画。HP バーなどの UI は `GameUIManager` が配置します。
  - ImGui: `imgui_impl_dx12`/`imgui_impl_win32` がコマンドを積み、同じコマンドリストへ統合します。
- **フレームの終了 (`Renderer::EndFrame`)**
  - コマンドリストをクローズして GPU へ送信し、スワップチェーンを Present。フェンスでフレーム同期を行い、次フレームのリソース更新を安全に行います。

## プロジェクト構成
- `SoccoreGameProject.sln` … Visual Studio 用ソリューション。
- `Main.cpp` … アプリのエントリーポイント。`ApplicationManager` が Win32 ウィンドウを作成し、`EngineManager` を起動します。
- `EngineManager.{h,cpp}` … DX12 リソース初期化、ImGui 設定、シーン管理のハブ。
- `Scene` / `StartScene` / `GameScene` / `GameOverScene` … シーンのライフサイクルを定義する基底クラスと実装。
- `Renderer` … 定数バッファ更新と 3D/2D 描画を司るクラス。
- `ObjectFactory` … キューブやスカイドーム、FBX スキニングモデル、リールなどゲーム内オブジェクトを生成。
- `Component` 群 … `Transform`、`Animator`、`Collider`、`Player1Component`/`Player2Component`、`ReelComponent`/`ReelController` などのゲームロジック。
- `assets/` … シェーダー (`.cso`)、テクスチャ、FBX モデル、Lua スクリプトなどのゲームリソース。

## ビルドと実行
1. Windows 10 以降と Visual Studio 2019/2022 (Desktop development with C++)、Windows 10 SDK を用意します。
2. `SoccoreGameProject.sln` を開き、`Debug` または `Release` 構成を選択してビルドします。
3. 実行すると 1280x720 のウィンドウが立ち上がり、Start 画面が表示されます。Enter キーで `GameScene` に遷移します。

## 操作の目安
- Start 画面: Enter キーでゲーム開始。
- プレイヤー 1: `J` でパンチ、`K` でキック、`G` でガード。Lua スクリプトで HP や移動速度をホットリロード可能です。
- 画面上部には HP バーが表示され、スロットリールとステージオブジェクトは自動で回転・更新されます。

## 開発時のヒント
- ImGui デバッグウィンドウは自動で初期化されます。`GameScene::Update` の `Mouse Debug` ウィンドウを参考にカスタム表示を追加できます。
- 定数バッファ (`BufferManager`) は 256 バイト境界で確保しているので、描画オブジェクト数を増やす際は必要サイズを調整してください。
- リソースやコンポーネントを増やす場合は `ObjectFactory` と `GameScene::m_SceneObjects` への登録を忘れずに行ってください。