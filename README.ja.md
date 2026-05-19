# Virtual Desktop Switcher for Windows 11
Windows 11 の仮想デスクトップをマウスで切り替えるツールです。

[English README](README.md)


## インストール方法
- https://github.com/saltpp/VirtualDesktopSwitcher/releases から zip ファイルをダウンロードして解凍してください
- VDS.exe, VDS.ini, VirtualDesktopAccessor.dll を任意のディレクトリに置いてください
- VDS.exe を起動すると、↔ アイコンがタスクトレイに追加されます


## アンインストール方法
- VDS.exe, VDS.ini, VirtualDesktopAccessor.dll を削除してください
- レジストリは使用してないです


## 使用方法
- マウスカーソルを画面の左右の端に置くと仮想デスクトップを切り替えれます
- このアプリを使う前に複数の仮想デスクトップを作っておく必要があります。（[Win]+[Tab] → [新しいデスクトップ] or [Ctrl]+[Win]+[d]）


## 注意
- 初回起動時に Windows Defender SmartScreen の警告が表示される場合があります。その場合は、「詳細情報」→「実行」を選択してください。


## 謝辞
- このアプリは、[VirtualDesktopAccessor](https://github.com/Ciantic/VirtualDesktopAccessor) ライブラリを使用しています。


## 履歴
- Ver.1.3.1
  - ピン設定されたウィンドウをドラッグして仮想画面を切り替えるとピン設定が外れる問題を修正

- Ver.1.3.0
  - [VirtualDesktopAccessor](https://github.com/Ciantic/VirtualDesktopAccessor) ライブラリを利用することで、Windows11 に対応

- Ver.1.2.0
  - log 表示部分の変更
  - README.md/.html の変更

- Ver.1.1.0
  - ウィンドウをドラッグしながら仮想デスクトップを切り替える機能を追加

- Ver.1.0.0
  - 最初のリリース


## ライセンス
- MIT
