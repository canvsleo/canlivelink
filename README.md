# ue4-3dsMax-livelink
---

　3ds Max でも Unreal Engine の Live Link Plugin の機能を使用できるようにして、  
若干の機能拡張を行ったプラグインです。


## 機能概要

以下の機能が実装されています。

* 3ds Max との Live Link 機能の対応
* 通常の Live Link Plugin のビューにてサポートしていない画角同期への対応
* 3ds Max 上でのスケルタルメッシュデータを同期する機能の追加


## 使用方法

　通常の「Live Link Perview Controller」と使用方法は変わりません。  
メッシュのプレビューシーンの「Preview Controller」にて、  
プラグインを組み込んだ際に追加される  
「Live Link Extend Perview Controller」を選択することで動作します。

#### 3dsMax での操作

<p align="right" style="float:right">
![image](./readme/max_control_panel.gif)
</p>

* Subject  
    Live Link にて同期させる対象に関する項目です。

	テキストボックスにSubjectの名前を入力して、  
	「Add」ボタンを押すことで選択中、またはすべてのオブジェクトが  
	1つのSubjectとしてUE4への同期単位として追加されます。  

	リストボックスの内容を選択して「Remove」ボタンを押すと、  
	Subjectを破棄します。

* Link  
	同期方法に関する項目です。

	* Force Front X Axis
		X軸が前方向に設定されたスケルタルメッシュと同期する場合は  
		「Force Front X Axis」にチェックを入れてください。

	* Send Mesh
		「Send Mesh」ボタンを押すことで、  
		メッシュがプレビューに同期されます。
		> *同期はアセットの上書きとして行われますので、保存などにはご注意ください。*

	* Automatic Sync Mesh
		「Automatic Sync Mesh」にチェックを入れると、  
		メッシュデータの更新が行われるときに  
		自動でLive Linkプレビューへの反映も行われるようになります。  
		> *この項目はメッシュデータの大きさにも依存しますが、大きく負荷がかかる可能性がありますので、ご注意ください。*


* Mesh format  
	メッシュ同期に使用されるフォーマットの項目となります。

	* UV nums 
		頂点が使用するUVの数を指定できます。

	* Vertex Colors
		頂点カラーの数を指定できます。
		> *2まで設定できますが、現在は 0 と 1 のみ対応されています。*

<p style="clear:both"></p>

## インストール方法

#### 3ds Max へのインストール

　3dsMaxのバージョンに対応した、 plugin/3dsMax ～ の中の dlu ファイルを、  
プラグインフォルダに格納することでユーティリティとして使用できます。

　
#### Unread Engine 4 へのインストール



## ビルド方法



