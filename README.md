# BLE 対応心拍計(M5Stick-C 利用)

M5Stick-C + 自作センサーモジュールを BLE 対応心拍計として使用するためのソースコード.

概要、自作センサーモジュールなどは を参照。

## インストール

`$ pio run -e m5stick-c -t upload`

## ボンディング(ペアリング)

M5Stick-C からの出力を `$ pio device monitor -b 115200` 等でをモニターしながら BLE で接続すると、以下のようにランダムな PIN が表示されます.
接続側から PIN を入力することでボンディング(ペアリング)が完了します.

```
PIN(PassKey): 123456
```

## BLE デバイス名を変更

```
$ export BLE_DEVICE_NAME='BLE_DEVICE_NAME=\"my_device01\"'
$ pio run -e m5stick-c -t upload
```

## BLE PIN(PassKey) を静的に指定

```
$ export BLE_STATIC_PASS_KEY='BLE_STATIC_PASS_KEY=123456'
$ pio run -e m5stick-c -t upload
```

6 桁よりも少ない桁数を指定した場合、接続側での入力は 6 桁になるように先頭を `0` で埋める必要があるかもしれません(ie. `12345` で設定した場合、`012345` と入力)。

## スクリーンショット

以下のコマンドでスクリーンショット用のファームウエアへ入れ替え後、モニターを開始。

```
$ pio run -e m5stick-c_screen_shot -t upload
$ pio device monitor -b 115200 | grep -e "==== BMP:" | sed -e "s/==== BMP://" | awk '/[a-zA-Z0-9+\/=]/{ decode ="base64 -i -d | convert bmp:- example" NR ".png "; print $0 | decode ; close(decode) }' RS="---- cut ----"
```

モニターを実行中に以下の手順を実施。

- M5Stick-C の Button B をクリック。
- `Ctrl - C` 等でモニターを停止。
- `example*.png` が作成される。

※ 最後の1枚(1ファイル)は失敗することが多いので、確実にスクリーンショットを取得するならば、Button B を複数回クリックする方が無難です。
