<div align="center">

  <img src="https://github.com/FLUXXFALCON/esp32-nURF-BOX/blob/main/images/logo.png" alt="logo" width="100" height="auto" />

  <h1> nURFBOX </h1>
  <p> All-in-One Gadget for BLE and 2.4GHz Networks </p>


<!-- Badges -->
<a href="https://github.com/FLUXXFALCON/esp32-nURF-BOX" title="Go to GitHub repo"><img src="https://img.shields.io/static/v1?label=FLUXXFALCON&message=nurfbox&color=purple&logo=github" alt="FLUXXFALCON - nurfbox"></a>
![GitHub Downloads (all assets, all releases)](https://github.com/FLUXXFALCON/esp32-nURF-BOX)
<a href="https://github.com/cifertech/nrfbox"><img src="https://img.shields.io/github/stars/cifertech/nrfbox?style=social" alt="stars - nrfbox"></a>
<a href="https://github.com/cifertech/nrfbox"><img src="https://img.shields.io/github/forks/cifertech/nrfbox?style=social" alt="forks - nrfbox"></a>


<h4>
    <a href="https://twitter.com/techcifer">TWITTER</a>
  <span> · </span>
    <a href="https://www.instagram.com/cifertech/">INSTAGRAM</a>
  <span> · </span>
    <a href="https://www.youtube.com/@techcifer">YOUTUBE</a>
  <span> · </span>
    <a href="https://cifertech.net/">WEBSITE</a>
  </h4>
</div>
<br/>


# nURF-BOX

`nURF-BOX`, ESP32 tabanlı gelişmiş bir kablosuz araç kutusudur. Bu proje, birçok kablosuz protokol ve güvenlik aracını bir araya getirir. Bu araçlar arasında Wi-Fi analiz, Bluetooth jammer, Evil Twin saldırıları ve IR kontrol gibi işlevler bulunur.

## Özellikler

* **RF24 Modülleri**: 3 adet RF24 modülü ile geniş kablosuz iletişim.
* **NeoPixel LED**: Durum göstergeleri için LED ışıklar.
* **OLED Ekran**: Durum ve menü görüntüleme için 128x64 çözünürlük.
* **Çeşitli Modüller**: Wi-Fi Jammer, BLE Jammer, Signal Cloner, IR TV Control ve daha fazlası.
* **EEPROM Ayarları**: Ekran parlaklık ayarı için EEPROM depolama.
* **Buton Kontrollü Menü**: Kullanıcı dostu menü, butonlarla seçim yapılabilir.

## Kurulum

### Gereksinimler

* **ESP32**: Bu proje ESP32 mikrodenetleyicisini kullanır.
* **Arduino IDE**: Arduino IDE kullanarak projeyi yükleyebilirsiniz.
* **RF24 Kütüphanesi**: Kablosuz iletişim için RF24 kütüphanesi gereklidir.
* **Adafruit NeoPixel**: LED kontrolü için Adafruit NeoPixel kütüphanesi gereklidir.
* **U8g2lib**: OLED ekran için gerekli kütüphane.

### Kütüphanelerin Yüklenmesi

Arduino IDE'ye gitmek için `Sketch > Include Library > Manage Libraries...` menüsüne tıklayın ve aşağıdaki kütüphaneleri arayın:

* `RF24` - Kablosuz modül için.
* `Adafruit NeoPixel` - LED şerit için.
* `U8g2lib` - OLED ekran için.
* `IRremoteESP8266` - IR kontrol için.

### Bağlantı

ESP32'yi bilgisayarınıza bağlayın ve ardından Arduino IDE üzerinden projenizi ESP32'ye yükleyin.

## Kullanım

### Menüler

1. **Scanner**: Kablosuz ağları tarar.
2. **Analyzer**: Ağ analizleri yapar.
3. **WLAN Jammer**: WLAN sinyallerini engeller.
4. **Proto Kill**: Prototip saldırıları yapar.
5. **BLE Jammer**: Bluetooth Low Energy (BLE) sinyallerini engeller.
6. **BLE Spoofer**: BLE cihazları taklit eder.
7. **Sour Apple**: Kablosuz sinyalleri manipüle eder.
8. **BLE Scan**: BLE cihazlarını tarar.
9. **WiFi Scan**: Wi-Fi ağlarını tarar.
10. **About**: Hakkında bilgisi gösterir.
11. **Settings**: Ekran parlaklığı gibi ayarları yapar.
12. **Evil Twin**: Evil Twin saldırısı gerçekleştirir.
13. **Signal Cloner**: Sinyalleri kopyalar.
14. **IR TV Control**: IR ile TV kontrol eder.

### Buton Kontrolü

* **Yukarı Butonu**: Menüde yukarı kaydırır.
* **Aşağı Butonu**: Menüde aşağı kaydırır.
* **Seçim Butonu**: Seçilen menüyü açar.

## Katkıda Bulunma

Eğer bu projeye katkıda bulunmak isterseniz, aşağıdaki adımları izleyebilirsiniz:

1. Bu repoyu çatallayın (`fork`).
2. Yeni bir dal oluşturun (`branch`).
3. Değişikliklerinizi yapın ve test edin.
4. Değişikliklerinizi geri gönderin (`pull request`).

## Lisans

Bu proje [MIT Lisansı](LICENSE) ile lisanslanmıştır.

