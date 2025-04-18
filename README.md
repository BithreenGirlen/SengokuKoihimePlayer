# SengokuKoihimePlayer
某夜伽鑑賞用。

## 動作要件
- Windows 8 以降のWindows OS
- MSVC 2015-2022 (x64)

## 再生方法
以下のような階層から成るファイル群をご用意した上で、  
メニュー欄`Folder -> Open`から`adventure/sprite/stillAnimation/st_XXXXXXXX`フォルダを選択すると再生を開始します。
<pre>
...
├ adventure
│  ├ json // 台本フォルダ
│  │  ├ ...
│  │  ├ 1036500501.json
│  │  └ ...
│  └ sprite
│     ├ ...
│     └ stillAnimation // 静画フォルダ
│        ├ ...
│        ├ st_10365005 // この階層のフォルダを選択
│        │  ├ fin.jpg
│        │  ├ output1.jpg
│        │  ├  ...
│        │  ├ output14.jpg
│        │  ├ wait1.jpg
│        │  └ wait2.jpg
│        └ ...
├ audios
│  └ voice // 音声フォルダ
│     └ adv
│        ├ ...
│        ├ 1036500501
│        │  ├ vo_1036500501_36_1.mp3
│        │  └ ...
│        └ ...
└ ...
</pre>

## マウス機能
| 入力 | 機能 |
| ---- | ---- |
| マウスホイール | 拡大・縮小。|
| 左ボタン + マウスホイール| コマ送り加速・減速。静止画時機能なし。|
| 左ボタンドラッグ | 表示範囲移動。ディスプレイ解像度以上に拡大した場合のみ動作。|
| 中ボタン | 原寸大表示・コマ送り速度初期化。|
| 右ボタン + マウスホイール | 文章送り・戻し。|
| 右ボタン + 中ボタン | 窓枠消去・表示。消去時にはディスプレイ原点位置に移動。|
| 右ボタン + 左ボタンクリック | 窓移動。窓枠消去時のみ動作。|

## キー機能
| 入力  | 機能  |
| --- | --- |
| <kbd>Esc</kbd> | 終了。 |
| <kbd>C</kbd> | 文字色黒・白切り替え。 |
| <kbd>T</kbd> | 文章表示・非表示切り替え。 |
| <kbd>∧</kbd> | 前のフォルダに移動。 |
| <kbd>∨</kbd> | 次のフォルダに移動。 |
| <kbd>＞</kbd> | 文章送り。 |
| <kbd>＜</kbd> | 文章戻し。 |

## メニュー機能
| 分類 | 項目 | 機能 |
| ---- | ---- |---- 
| Folder | Open | フォルダ選択。
| Setting | Audio |  音量・音声再生速度設定。
| - | Font | 書体設定。
| Image | Pause | 自動コマ送り停止。左クリックで1コマずつ送る状態になります。

## 外部ライブラリ
- [JSON for Modern C++ v3.11.3](https://github.com/nlohmann/json/releases/tag/v3.11.3)

## 構築方法
1. `deps/CMakeLists.txt`を実行して外部ライブラリを取得。
2. Visual Studioから`SengokuKoihimePlayer.sln`を開く。
3. メニュー欄`ビルド`から`ソリューションのビルド`を選択。
