# M5Stack IRcube

M5Stackで赤外線による家電リモコンを実現します。

主要国内メーカーのTVをサポートしています。

本体画面のメニューから操作できるほか、Wi-Fi接続での操作にも対応します。

## 対応デバイス

M5Stackシリーズ + 赤外線ユニット

* M5Stack Officilal IR UNIT （[DEMO VIDEO](https://www.youtube.com/watch?v=DrlsMsi8R2U)）

* 雑赤外線モジュール （[DEMO VIDEO](https://www.youtube.com/watch?v=6TpuKI1cwZU)）

* 超雑赤外線モジュール （[DEMO VIDEO](https://www.youtube.com/watch?v=v7prP7vNd5k)）

M5Stack Grayにてテスト。

ピン番号を変えれば、その他のデバイスにも対応すると思われます。ご報告お待ちしております。

## 使い方

初めて電源を入れた時は接続するWiFiネットワークがされていませんが、本体操作でTVリモコンの操作が可能です。

Wi-Fi経由で操作したい場合は以下のとおりに。

LovyanLauncherなどで接続先を設定してある場合は、設定済みのアクセスポイントに接続します。

## Wi-Fi経由での操作の準備

設定が済んでいない場合は、起動後に「0.0.0.0」と右下に表示されます。

その場合は、Cボタンを押しながらリセット。

アクセスポイントモードで起動するので、スマホなどで「 M5Stack-WiFi 」に接続。

ウェブブラウザで「 192.168.4.1 」を開き、使用しているWiFiルーター/アクセスポイントなどのSSIDとパスワードを入力してOk。再起動すると、M5StackCが指定したWiFiネットワークに接続されます。

この時点でスマホのWiFi設定を元に戻します。

M5Stack本体のLCDに表示されるIPアドレスにウェブブラウザでアクセス。データ送信が行えます。

LEDのON/OFFのボタンは擬似的なもの（動作確認のために残しています）。

送信データはIRKkit互換。

送信データの作成はM5Stack本体だけだと無理なので、各社TVを操作できるウェブアプリを作りました。

http://m5.linclip.com/ir/


## 本体ボタンについて

A、B、Cボタンでツリー的なメニューを操作します。

メニューは[LovyanLauncher](https://github.com/lovyan03/M5Stack_LovyanLauncher)、[M5Stack_TreeView](https://github.com/lovyan03/M5Stack_TreeView) と同様。


## 参考にしたもの

* [minlRum](https://github.com/9SQ/minIRum)

* [IRKit](http://getirkit.com/)

特に、データ送信に関してはminlRumのコードをそのまま使用させていただきました。

## 使用ライブラリ

* https://github.com/Brunez3BD/WIFIMANAGER-ESP32

* https://github.com/interactive-matter/aJson （要パッチ https://gitlab.com/xarduino/lightsw/blob/master/patch/ajson-void-flush.patch）

* https://github.com/SensorsIot/Arduino-IRremote


