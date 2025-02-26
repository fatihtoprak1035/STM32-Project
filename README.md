# 🚀 STM32F401RET6 Tabanlı Otomatik ve Manuel Kontrol Sistemi

Bu proje, **STM32F401RET6** mikrodenetleyicisini kullanarak çeşitli sensörler ve donanım bileşenleri arasında **uyumlu bir kontrol sistemi** oluşturmayı amaçlamaktadır. Sistem, **otomatik ve manuel modlarda** çalışarak gerçek dünya senaryolarında test edilmiştir.

---

## 📌 Uygulama Amacı
Bu uygulamanın amacı, **mesafe algılama, motor kontrolü, görsel ve işitsel geri bildirimler ve kullanıcı etkileşimini** birleştirerek, sistemin **kararlı bir şekilde** çalışmasını sağlamaktır.

## ⚙️ Sistem Çalışma Prensibi
Sistem iki farklı modda çalışır:

### **1️⃣ Otomatik Mod (Varsayılan Başlangıç Modu)**
Bu mod, sistem açıldığında **otomatik olarak** etkinleşir. **Mesafe sensörü**, çevredeki mesafeyi algılayarak sistemin davranışını belirler.

- **Eğer mesafe 10 cm'den küçükse:**
  - RGB LED **kırmızı** yanar.
  - **Buzzer aktif olur** (sesli uyarı verir).
  - **Motor ileri döner**.
  - **LCD ekranında şu bilgiler görüntülenir:**
    ```
    DIST: NEAR  
    MOTOR: FORWARD  
    ```

- **Eğer mesafe 100 cm’den büyükse:**
  - RGB LED **yeşil** yanar.
  - **Buzzer pasif olur**.
  - **Motor geri döner**.
  - **LCD ekranında şu bilgiler görüntülenir:**
    ```
    DIST: FAR  
    MOTOR: BACKWARD  
    ```

### **2️⃣ Manuel Mod (Butona Basıldığında Etkinleşir)**
Bu mod, **sistemde bulunan butona basıldığında** aktif olur. **Potansiyometreden** alınan verilere göre motorun yönü ayarlanır.

- **Eğer potansiyometre belirli bir eşik değerinin altındaysa:**
  - **Motor ileri döner**.
  - **LCD ekranında şu bilgiler görüntülenir:**
    ```
    MANUAL MODE  
    MOTOR: FORWARD  
    ```

- **Eğer potansiyometre belirli bir eşik değerinin üzerine çıkarsa:**
  - **Motor geri döner**.
  - **LCD ekranında şu bilgiler görüntülenir:**
    ```
    MANUAL MODE  
    MOTOR: BACKWARD  
    ```

📢 **Her iki modda da, sistemin durumu ve yönü LCD ekranı aracılığıyla kullanıcıya iletilir.**

---

## 🛠️ Kullanılan Donanım ve Bileşenler
- **Mikrodenetleyici:** STM32F401RET6
- **Sensörler:** Ultrasonik mesafe sensörü (HC-SR04 veya benzeri)
- **Motor Sürücü:** L298N veya benzeri motor sürücü kartı
- **Motor:** DC Motor
- **Görsel ve İşitsel Uyarılar:** RGB LED ve Buzzer
- **Kullanıcı Girişi:** Buton ve Potansiyometre
- **Ekran:** LCD 16x2 veya OLED ekran

---

## 🚀 Projeyi Çalıştırma
1. **STM32CubeIDE veya Keil uVision** kullanarak projeyi açın.
2. Donanım bağlantılarını **şemaya uygun** şekilde yapın.
3. **Kodları derleyin ve STM32F401RET6 mikrodenetleyicisine yükleyin.**
4. **Sistemi başlatarak, otomatik ve manuel modları test edin.**

---

## 📜 Lisans
Bu proje **MIT Lisansı** ile lisanslanmıştır. Özgürce kullanabilir, geliştirebilir ve paylaşabilirsiniz.

📧 **İletişim:** Proje hakkında sorularınız varsa, GitHub üzerinden issue açabilirsiniz! 🚀

