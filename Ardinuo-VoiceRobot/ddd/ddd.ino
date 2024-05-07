// Televizyon Modu
#include <IRremote.h>//Televizyon Kızılötesi(IR) Sinyali Kütüphanesi
IRsend irsend; // ir sinyali göndereceğimiz değişken

const int led = 9; // ir sinyalini gönderecek led port numarasını atıyoruz.
// const sabit bir değişken olduğunu gösterir.

// sayı kumanda hex kodları
#define sifir 0x80BFE11E
#define bir 0x80BF49B6
#define iki 0x80BFC936
#define uc  0x80BF33CC
#define dort 0x80BF718E
#define bes 0x80BFF10E
#define alti 0x80BF13EC
#define yedi 0x80BF51AE
#define sekiz 0x80BFD12E
#define dokuz 0x80BF23DC

//yonetim kumanda hex kodları
#define ses_ileri 0x80BF837C
#define ses_geri 0x80BF9966
#define ileri 0x80BF53AC



//sıcaklık sensörü
int sensor_deger = 0; // sensörden direk okunan değer
int sicaklik_deger = 0; // hesaplandıktan sonraki net değer(°C)
float voltaj_deger = 0; // sensördeki anlık voltaj değeri


//LCD
#include <LiquidCrystal.h> // lcd kütüphanesi
LiquidCrystal lcd(26,28,30,32,34,36);//rs,e,d4,d5,d6,d7
//SD Kart
#include <SD.h>   //SD kart kütüphanesi
#define SD_ChipSelectPin 53  // sd kart için veri alış-verişi 53.pin
#include <SPI.h> // sd kart kütüphanesi

#include <TMRpcm.h>           //  ses çalma kütüphanesi
TMRpcm sound_player;   // ses çalma değişkeni


bool yeni_muzik=true; 
String gecerli_gorev="";

// cihaza gelen sesi tutacağımız değişken
String voice = "";
// cihaz moda sahip mi
bool mod=false;

void setup() {
   sound_player.speakerPin = 11; // Ardinuo ses çıkışı pini

  Serial.begin(9600);// Serial Portu 9600 bound değerinde başlatıyoruz
  pinMode(led,OUTPUT); // ir led çıkışı(aktif etme)
  
  if (!SD.begin(SD_ChipSelectPin)) { //sd kart için seçilen pine sd kart modülü bağlı mı ?
    Serial.println("SD fail");   // durum negatifse hata mesajını seri porta yazıdır
    return;  //geri değer döndür boş olarak
  }
 
  

lcd.begin(16, 2); // 16. ve 2. pinleri aktif et
lcd.setCursor(2,0); // 2.hane, 0.satırdan başla
lcd.print("Voice Robot"); // ilgili yazıyı yazma
lcd.setCursor(1,1);// 1.hane 1.satırdan başla
lcd.print("Hos Geldiniz :)"); // ilgili yazıyı yaz


}

void loop() {


  // serial port haberleşmesi ve sesin karakter karakter okunup
  // voice değişkeninde toplanması
  while (Serial.available()){ // seri haberleşme bitene kadar
  delay(10);// 10 milisaniye bekle
  char c = Serial.read();// karakter
  if (c == '#') {break;}//# işaretine eşitse karakter seri haberleşmenin sonuna gelmişiz.
  voice += c;// karakteri değişken içine atıyoruz
  voice.toLowerCase(); // tüm karakterleri küçült
  voice.replace(" ","_");//boşlukların hepsini _ işareti ile değiştir
  }


  // komut uzunluğu 0 dan büyükse komut vardır
  if (voice.length() > 0) {
      Serial.println(voice);// varsa serial ekranında göster      
          
            
     if(gecerli_gorev=="" && !search(voice,"modu") && !search(voice,"sıcaklık_kaç_derece")){
      // muzik çalmıyorsa
      if(sound_player.isPlaying()== 0) {SoundPlay("uyarilar","herhangi_bir_modda_calismiyorum",0);}
     }
     // görev ataması metodu
     ModAtama(voice);
   
  // sesi aldık kullandık işi bitti
  voice="";
  }
  
}
// Tv Fonksyonları
void TvKumandasi(String gelen_komut){
  
  bool kanal=true;
  //Kumanda Algoritması
  if(!search(gelen_komut,"modu") && !search(gelen_komut,"moduna")){

       // özellik tuşları 
       if(gelen_komut=="ses_ileri") {kanal=false;  irsend.sendNEC(ses_ileri,32);} else
       if(gelen_komut=="ses_geri") {kanal=false;irsend.sendNEC(ses_geri,32);}
    if(kanal){   
      // tv_kods.txt dosyasında gelen komutun idsini(kanal no) bul getir
       String tv_id=db_dataToID("tv_kods",gelen_komut);

       
       if(tv_id.length()==1){
          //tv_id'nin 0.indexinden başla 1 sayı alarak
          //metoda gönderilmesini sağlıyoruz.
           TvTusaBas(tv_id.substring(0,1));
            
            
       }else
       if(tv_id.length()==2){

        //Seri ekranda iki haneli olduğunu görmek istiyorum.
        Serial.println("iki haneli: "+tv_id.substring(2,1));

         //birinci hanesini al ve gönder
         TvTusaBas(tv_id.substring(0,1));
         delay(100);// 100 milisaniye bekle
          //(deneme yanılma ile keşfettim)
          // arda arda basılırken gerekli hex kodu
         irsend.sendNEC(0xFFFFFFFF,32);
         delay(100);// 100 milisaniye bekle

         // ikincisi hanesini al ve gönder
         TvTusaBas(tv_id.substring(2,1));
         //(deneme yanılma ile buldum)
         // ileri tuşuna basmassak bu kombinasyon çalışmıyor
         irsend.sendNEC(ileri,32);  
       }else
       if(tv_id.length()==3){
         Serial.println("üç haneli: "+tv_id.substring(5,2));
         //birinci hanesini al ve gönder
         TvTusaBas(tv_id.substring(0,1));
         delay(100);// 100 milisaniye bekle
         // arda arda basılırken gerekli hex kodu
         irsend.sendNEC(0xFFFFFFFF,32);
         delay(100);// 100 milisaniye bekle
         // ikincisi hanesini al ve gönder
         TvTusaBas(tv_id.substring(2,1));
         delay(100);// 100 milisaniye bekle
          // üçüncü hanesini al ve gönder
         TvTusaBas(tv_id.substring(5,2));
         irsend.sendNEC(ileri,32);  
       }
        //Lcd ekranın alt satırına kanal ismini yazdıralım
        LcdAltSabit("Kanal: "+gelen_komut);
    }
  }
       
}
void TvTusaBas(String gelen_tv_id){
      // gelen sayıya göre ilgili ir kodu gönderiliyor.
  
            if(gelen_tv_id=="0")
              irsend.sendNEC(sifir,32);
            else  
            if(gelen_tv_id=="1")
              irsend.sendNEC(bir,32);
            else
            if(gelen_tv_id=="2"){
              irsend.sendNEC(iki,32);
            }else
            if(gelen_tv_id=="3"){
              irsend.sendNEC(uc,32);
            }else
            if(gelen_tv_id=="4"){
              irsend.sendNEC(dort,32);
            }else
            if(gelen_tv_id=="5"){
            irsend.sendNEC(bes,32);
            }else
            if(gelen_tv_id=="6"){
            irsend.sendNEC(alti,32);
            }else
            if(gelen_tv_id=="7"){
            irsend.sendNEC(yedi,32);
            }else
            if(gelen_tv_id=="8"){
            irsend.sendNEC(sekiz,32);
            }else
            if(gelen_tv_id=="9"){
            irsend.sendNEC(dokuz,32);
            }
}
//Tv Fonskyonları son
void ModAtama(String gelen_komut){
   Serial.println("Gelen Komut: "+gelen_komut);
  if(search(gelen_komut,"modu") || search(gelen_komut,"moduna") || search(gelen_komut,"sıcaklık_kaç_derece")){

    // gelen komutta sıcaklık kaç derece ise
    if(search(gelen_komut,"sıcaklık_kaç_derece")){
      // geçerli görev ata
      gecerli_gorev="sicaklik_ogrenme_modu";
      // seri ekrana yazdır ki daha iyi anlaşılması için
      Serial.println("Sıcaklık Öğrenme Modu Aktif");
      // cihaz artık bir moda sahip
      mod=true;
    }else
    // gelen komut muzik çalma modu ise
    if(search(gelen_komut,"müzik_çalma_modu")){
        // geçerli görev ata
        gecerli_gorev="müzik_çalma_modu";
        //seri ekrana yazdır ki daha iyi anlaşılması için
        Serial.println("Müzik Çalma Modu Aktif :)");
        //muzik çalma modu aktif sesini çal
        SoundPlay("modlar","muzik_calma_modu_aktif",0);
        // Lcd ekranın üst kısmına yazdır
         LcdUstSabit(gecerli_gorev);
        //cihaz artık bir moda sahip
        mod=true;
    }   
    else
    if(search(gelen_komut,"televizyon_modu")){
        // geçerli görev ata
        gecerli_gorev="televizyon_modu";
        //seri ekrana yazdır ki daha iyi anlaşılması için
        Serial.println("Televizyon Kumandası Modu Aktif :)");
        //televizyon modu aktif sesini çal
        SoundPlay("modlar","televizyon_modu_aktif",0);
        //Lcd üst kısma Tv Kumandası Mod yazdır
        LcdUstSabit("Tv Kumandası Mod");
        // cihaz artık bir moda sahip
        mod=true;
    }
   
  }else
  // geçerli görev muzik çalma moduysa.
  if(gecerli_gorev=="müzik_çalma_modu"){
    // burası çok uzun olduğu için ayrı bir metodda tutuyoruz.
    YeniMuzik(gelen_komut);
    
  }
  // geçerli görev sıcaklık öğrenme moduysa
  if(gecerli_gorev=="sicaklik_ogrenme_modu"){ 
        sensor_deger = analogRead(0); // sensör değeri al 0. analog pinden
        voltaj_deger = (sensor_deger / 1023.0) * 5000 ; 
        sicaklik_deger = voltaj_deger / 10.0;
        sicaklik_deger = (sicaklik_deger * 2);

        //Sıcaklık değerini Serial ekranına yazdırıyoruz.
        Serial.println("Sıcaklık değeri: "+String(sicaklik_deger));

        // cihazımız artık bir moda sahip
       mod=true;
       // cdegers.txt bilgisiyle sicaklık değerini 0.saniyeden itibaren çal
       SoundPlay("cdegers",String((sicaklik_deger)),0);
       // geçerli görev ortamın sıcaklığı olacağı için bunu ata
       gecerli_gorev="Ortam Sıcaklıgı";
       // Lcd ekranın alt satırana sıcaklık değerini yazdır
       LcdAltSabit(String((sicaklik_deger))+" derece");
       
  }
  // geçerli görev televizyon modu
  if(gecerli_gorev=="televizyon_modu"){

   
    TvKumandasi(gelen_komut); 
  }

  
  if(!mod){ // geçerli moda sahip değilse
       if(sound_player.isPlaying()== 0) SoundPlay("uyarilar","boyle_bir_moda_sahip_değilim",0);  
  }
}



bool search(String aranilan, String aranan) {
    // aranilan değerin uzunluğundan aranan değeri çıkarıyoruz.
    int max = aranilan.length() - aranan.length();

    for (int i = 0; i <= max; i++) {// max değere gelene kadar arttır
        if (aranilan.substring(i) == aranan) return true; //aranılan aranana değere eşitse
    }

    return false; //değilse false
}

String komutTemizligi(String gelen_komut_dizisi[],String gelen_komut){

  // gelen_komut_dizisi arınması gereken kelime listesi
  // gelen_komut ise arınacak kelime grubu

  // gelen_komut dizisinin uzunluğu kadar döngümüzü başlatıyoruz
 for (int i = 0; i < (sizeof(gelen_komut_dizisi)/sizeof(int)); i++) {
      // eger gelen komut içerisinde dizinin ilgili indexdeki
      // değeri var ise
      if(search(gelen_komut,gelen_komut_dizisi[i])){
      // bu değeri nerde gördüysen bulup "" ile temizle  
        gelen_komut.replace(gelen_komut_dizisi[i],"");
      }
  }
  // temizlenmiş komutu geri döndür
  return gelen_komut;
}

String db_dataToID(String dosya_ismi,String search_text){

  //Serial.println("Aranacak veri: "+search_text);
  
  String id="bos"; // geri dönecek id değişkeni
  // gerekli format
  dosya_ismi.concat(".txt"); // dosya ismine .txt uzantısı ekliyoruz

  // dosya ismimizi char karakter formatına dönüştürüyoruz.
  char dosya_ismi_format[dosya_ismi.length()+1];
  dosya_ismi.toCharArray(dosya_ismi_format, sizeof(dosya_ismi_format));

  
  File dosya; // File tipinde bir değişken oluşturuyoruz
  String satir; // satırını komple tutacağımız değişken
  String satir_data; // satırdaki
  String aranacak_kelime; // satırda aranacak veri
 
  
  /*dosya ismimizi formatlanmış bir şekilde SD kart kütüphanesinin open
    metoduna gönderiyoruz ve bize txt dosyamızı veriyor.
  */
  dosya = SD.open(dosya_ismi_format);
  if (dosya) { // dosya tamam ise
    
    while (dosya.available()) { // veri olmayana kadar okumaya devam et
      satir = String(dosya.readStringUntil('\n')); // boşluğa gelince kes(satır)
      satir_data=String(db_idTodata_parcala(satir)); // satırdaki veriyi parçala
      satir_data.trim();// satırdatanın sonundaki boşluğu kırpıyoruz
      /*aranacak kelime, metota parametre olarak geliyor bu kelimeyi
        karakterlerini küçültüp ingilizceye çeviriyoruz. Yoksa satır içinde
        arama yapamayız. Eşleşme yapılamıyor
       */
      aranacak_kelime=String(trtoeng(search_text));

       // kelimeyi search metodu yardımıyla arıyoruz ve eğer satırda var ise
       // true dönüyo 
      if(search(satir_data,aranacak_kelime)){
        // ilgili veriyi bulduğumuz satırdaki idyi almalıyız  
        id=db_dataToid_parcala(satir);
        break;
       
      }
      
    }
    //dosyayla işimiz bitti kapatalım..
    dosya.close();
  } else {
     // dosya açılmaz ise
    Serial.println(""+dosya_ismi+".txt dosyası açılamadı !");
  }
  // id değerini geri döndürüyoruz
  return id;
}
String db_IDToData(String dosya_ismi,String gelen_id){

  //Serial.println("Aranacak veri: "+search_text);
  
  String data="bos";
  // gerekli format
  dosya_ismi.concat(".txt");

  char dosya_ismi_format[dosya_ismi.length()+1];
  dosya_ismi.toCharArray(dosya_ismi_format, sizeof(dosya_ismi_format));
  
  File myFile;
  String line;
  String line_data;
  String aranacak_id;

  if(sound_player.isPlaying()==1){ sound_player.stopPlayback();}
  
  myFile = SD.open(dosya_ismi_format);
  if (myFile) {
       //Serial.println("Dosya Açıldı");
    // read from the file until there's nothing else in it:
    while (myFile.available()) {

      line = String(myFile.readStringUntil('\n')); 
      line_data="enes";
      line_data=String(db_dataToid_parcala(line));
      line_data.trim();
      aranacak_id=String(trtoeng(gelen_id));

        
      if(search(line_data,aranacak_id)){  
        data=db_idTodata_parcala(line);
         //Serial.println("Eşleşen Satır: "+line+" ve numarası: "+id);
        
        break;
       
      }
      
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening "+dosya_ismi+".txt");
  }

  return data;
}

String db_dataToid_parcala(String parcalancak_veri){

    // verimizi iki parçaya bölücez değişlenlerimizi tanımlıyoruz  
    String id,icerik ; 
     

    // parcalancak verinin karakter sayısı kadar döngü başlatıyoruz
    for (int i = 0; i < parcalancak_veri.length(); i++) {
      // parcalancak vernin karakterlerini birer birer bakıyoruz
      if (parcalancak_veri.substring(i, i+1) == ",") {// virgüle eşite
        id = parcalancak_veri.substring(0, i);//virgülden önceki parçayı alıyoruz
        icerik= parcalancak_veri.substring(i+1);//virgülden sonraki parçayı alıyoruz
        break;
      }
  }

  return id;
}
String db_idTodata_parcala(String parcalancak_veri){

     //parçalancak veri ikiye ayrılacak  
    String id, veri; // bunun için değişkenlerimizi oluşturuyoruz
     
    // verinin harf harf okicak şekilde döngü oluşturuyoruz
    for (int i = 0; i < parcalancak_veri.length(); i++) {// değişkenin uzunluğuna gelene kadar
      if (parcalancak_veri.substring(i, i+1) == ",") {// birer birer seç ve virgüle eşitse
        id = parcalancak_veri.substring(0, i);// virgüle eşit olana kadarki kısım id
        veri= parcalancak_veri.substring(i+1); // virgülden sonraki kısım veri
        break; // döngümüzden çıkıyoruz.
      }
  }

  return veri; // bulduğumuz veriyi geri gönderiyoruz
}
String SoundSeekValue(String parcalancak_veri){

    //saniye değerini tutacağımız değişken
    String value;

    // metoda gelen değerin uzunluğu kadar döngümüzü oluşturuyoruz.
    for (int i = 0; i < parcalancak_veri.length(); i++) {
      //kelimenin birer birer ":" işaretine kadar
      if (parcalancak_veri.substring(i, i+1) == ":") { //":" gelene kadar devam et
        value= parcalancak_veri.substring(i+1); // ":" işaretinden sonrasını al
        break;
      }
  }
   // ayıklanan değeri geri döndür
  return value;
}
String SoundSeekName(String parcalancak_veri){

     //Şarkının ismini tutacağımız değişken
    String Name;
     
    // metoda gelen değerin uzunluğu kadar döngümüzü oluşturuyoruz.
    for (int i = 0; i < parcalancak_veri.length(); i++) {
      if (parcalancak_veri.substring(i, i+1) == "-") { // "-" işaretine eşit oluncaya kadar
        Name = parcalancak_veri.substring(0, i);// "-" işaretinden öncesini al
        break;
      }
  }
  // şarkının ismini geri döndürüyoruz.
  return Name;
}
String SoundPlay(String dosya_ismi,String gelen_komut,int baslangic){
  // burada gelen verileri adım adım muzik_kodu formatına çevirip çalıyoruz.
   String muzik_kodu=dosya_ismi; // dosya_ismini değişkene atıyoruz
   muzik_kodu.concat("/"); // dosya ismine slash işareti ekliyoruz.
   muzik_kodu.concat(db_dataToID(dosya_ismi,gelen_komut));      // ardından dosya yolunu buluyoruz

   muzik_kodu.concat(".wav"); // dosya uzantısını ekliyoruz
   char muzik_kodu_format[muzik_kodu.length()+1]; // muzik kodu karakter formatı oluşturuyoz
   //muzik kodunu char karakter dizine dönüştürüyoruz
   muzik_kodu.toCharArray(muzik_kodu_format, sizeof(muzik_kodu_format));
  
  // Not: Bu formata dönüştürmemizin sebebi müziğin çalmamasıdır.

  //Müziği Gloabal olarak tanımladığımız sound_player nesnesinin play metodu ile başlatıyoruz.
  sound_player.play(muzik_kodu_format,baslangic);//Çalınacak muzik,Muziğin başladığı saniye
   
 
  /* Müzik kodu yolunu geri döndürmek istedim.
   * Çünkü geri dönen değerin boş olup olmadığını anlamak istedim.
   */
  return muzik_kodu;
}

String trtoeng(String gelenveri){
    //buraya parametre ile gelen değer .replace metodu ile eski,yeni
    //şeklinde arananların hepsini bulup değiştiriyor
   gelenveri.replace("Ç","c");  
   gelenveri.replace("İ","i");
   gelenveri.replace("Ü","ü");
   gelenveri.replace("Ş","s");
   gelenveri.replace("Ö","o");
  
   gelenveri.replace("ı","i");
   gelenveri.replace("ğ","g");
   gelenveri.replace("ü","u");
   gelenveri.replace("ş","s");
   gelenveri.replace("ö","o");
   gelenveri.replace("ç","c");
   return gelenveri; // geri döndürüyor
}

void LcdUstSabit(String gelen_yazi){

  //gelen_yazi komut olacağı için düzgün temiz bir yazıya
  //çevirmek için tüm alttireleri boşluk yap.
  gelen_yazi.replace("_"," ");
  
  lcd.clear(); // ekranda yazı varsa temizle
  lcd.setCursor(0,0);//ekranın 0. satır 0.sütünuna
  lcd.print(trtoeng(gelen_yazi)); // açıklıyacağım burayı
}

void LcdAltSabit(String gelen_yazi){

  
    Serial.println("Dışarda gelen yazı: "+gelen_yazi);
  
  gelen_yazi.replace("_"," ");
  while(sound_player.isPlaying()==1 || gelen_yazi.length()>0){ // program kapanana kadar
    
   voice="" ;
   //Bluetooth Modülü veri okuma
  while (Serial.available()){
  delay(10);
  char c = Serial.read();
  if (c == '#') {break;}
  voice += c;
  voice.toLowerCase();
  voice.replace(" ","_");
  }
  Serial.println("İçerde ses: "+voice);
   if(voice=="şarkıyı_durdur" || voice=="müziği_durdur" || voice=="dur_dur" || voice=="şarkıyı_oynat" || voice=="şarkıyı_çal" || voice=="devam_et" || voice=="çal" || voice=="oynat"){
      yeni_muzik=false;
      sound_player.pause();
    }
    if(search(voice,"ses_arttir")){
      yeni_muzik=false;
      sound_player.volume(1);   
    }
    if(search(voice,"ses_azalt")){
      yeni_muzik=false;
      sound_player.volume(0);
    }
    if(voice=="" || voice=="şarkıyı_durdur" || voice=="müziği_durdur" || voice=="dur_dur" || voice=="şarkıyı_oynat" || voice=="şarkıyı_çal" || voice=="devam_et" || voice=="çal" || voice=="oynat" || search(voice,"ses_arttir") || search(voice,"ses_azalt") ){
      lcd.clear();
      LcdUstSabit(gecerli_gorev);
      lcd.setCursor(0,1);
      lcd.print(trtoeng(gelen_yazi.substring(0,17)));
      delay(500);
      if(gelen_yazi.substring(16,33).length()!=0){
        lcd.clear();
        LcdUstSabit(gecerli_gorev);
        lcd.setCursor(0,1);
        lcd.print(trtoeng(gelen_yazi.substring(16,33)));
        delay(500);
      }
      if(gelen_yazi.substring(33,48).length()!=0){
        lcd.clear();
        LcdUstSabit(gecerli_gorev);
        lcd.setCursor(0,1);
        lcd.print(trtoeng(gelen_yazi.substring(33,48)));
        delay(500);
      }
    } else {
      lcd.clear();
      
        ModAtama(voice); // gelen sese göre herşey başa dönsün
       break; 
      }
  }
}
void YeniMuzik(String gelen_komut){
    
    
    Serial.println("Müziğe geleen temiz:"+gelen_komut);
    
    if(gelen_komut=="şarkıyı_durdur" || gelen_komut=="müziği_durdur" || gelen_komut=="dur_dur" || gelen_komut=="şarkıyı_oynat" || gelen_komut=="şarkıyı_çal" || gelen_komut=="devam_et" || gelen_komut=="çal" || gelen_komut=="oynat"){
      yeni_muzik=false;
      sound_player.pause();
    }else if(gelen_komut.indexOf(":") !=-1){
      Serial.println("Doğru yerdesin: "+gelen_komut);
      int seekValue=SoundSeekValue(gelen_komut).toInt();
      String soundseekname=SoundSeekName(gelen_komut);
      yeni_muzik=false;
      Serial.println("Şarkı ismi:"+soundseekname);
      Serial.println("Düzgün başlangıç:"+seekValue);
      soundseekname.trim();
      /*
      if(seekValue==0) seekValue=0; else
      if(seekValue>0 && 13624 <=seekValue)seekValue=5;else
      if(seekValue>13624 && 19869 <=seekValue) seekValue=10;else
      if(seekValue>19869 && 23843 <=seekValue) seekValue=13;else
      if(seekValue>23843 && 27817 <=seekValue) seekValue=16;else
      if(seekValue>27817 && 34865 <=seekValue) seekValue=22;else
      if(seekValue > 34865) seekValue=30;
      
      Serial.println("Düzgün başlangıç:"+seekValue);
      
      */
      SoundPlay("muzikler",soundseekname,seekValue);   
      LcdAltSabit(soundseekname); 
      
    }
    else{yeni_muzik=true;} 
       
    if(yeni_muzik){// yeni bir muzikse
 /*
    if(gelen_komut=="sonraki_şarkı" || gelen_komut=="sonraki_müzik" ){
  
       String sonrakidata=db_IDToData("muzikler",int(gelen_komut) + 1);
       gelen_komut=sonrakidata; 
       Serial.println("Sonraki Şarkı İsmi:"+gelen_komut);
    }
    */
     String komut_temizle[4]={"_çal","_oynat","_dinle","_aç"};
    gelen_komut=komutTemizligi(komut_temizle,gelen_komut);
    
    String bosmu=SoundPlay("muzikler",gelen_komut,0);
    bosmu.trim();
    //Serial.println("Dönen: "+bosmu);
     
    if(search(bosmu,"muzikler/bos.wav")){Serial.println("Boş :)"); SoundPlay("uyarilar","böyle_bir_şarkı_bilmiyorum",0);} // muzik çalmıyorsa sıkıntı vardır
    //Serial.println("sayac: "+String(birkere_lcd));
    LcdAltSabit(gelen_komut);    
}
}







