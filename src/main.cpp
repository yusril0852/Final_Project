#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include <Arduino.h>
#include <math.h>
#include <UniversalTelegramBot.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

const int pH_p = 33;
const int Echo2_p = 23;
const int Trig2_p = 5;
const int Echo1_p = 18;
const int Trig1_p = 19;
const int Turbid_p = 32;
const int btn1 = 25;
const int btn2 = 26;
const int solesum_p = 4;
const int solebaw_p = 16;
const int solerum_p = 2;
const int sanyo_p = 17;

int switchstate = 0;
int solesum_state = 0;
int solebaw_state = 0;
int sanyo_state = 0;
static int hits = 0;
int prevswitchstate = 0;

float pH, ntu, volt, volttur;

int waktu;
int durasi = 0;
int waktuan, jaman, menitan, detikan;
int ibersv = 0;

float asam, netral, basa;
float sjernih, jernih, keruh;
float wbentar, wsedang, wlama, wasli;
float a1, a2, a3, a4, a5, a6, a7, a8, a9;
float r1, r2, r3, r4, r5, r6, r7, r8, r9;
String Validasikeadaantandon = "";
String Vkt = "";

// button
enum State
{
  monitor,
  bersih,
  monitorstatement,
  durasireset,
  penjadwalan
} Stateku;

// WebServer
const char *ssid = "Turangga";      // Wifi
const char *password = "xxxxx"; // Password
char server[] = "192.168.100.6";    // Server ipV4

// TELEGRAM BOT
#define BOTtoken "5xxxx"
#define CHAT_ID "xxxx"
WiFiClientSecure client;
int botRequestDelay = 500;
unsigned long lastTimeBotRan;
UniversalTelegramBot bot(BOTtoken, client);
int jaddat, jadmon, jadyea, jadsec, jadmin, jadhou;

// WiFiReconect
unsigned long pMilRec = 0;
unsigned long interRec = 30000;

// Sensor Ultrasonic
const unsigned int BAUD_RATE = 9600;
float WatLv, WatLv2;
float TankD = 74, TankD2 = 41;
float percentage, percentage2;
long dur, dur2;
int dis, dis2;

// LCD Setup
int lcdColumns = 20;
int lcdRows = 4;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Telegram Message Command

bool buttonB()
{ // fungsi baca button
  if (digitalRead(btn1) == LOW)
  {
    delay(100);
    while (digitalRead(btn1) == LOW)
      ;
    return 1;
  }
  else
    return 0;
}

bool buttonA()
{ // fungsi baca button
  if (digitalRead(btn2) == LOW)
  {
    delay(100);
    while (digitalRead(btn2) == LOW)
      ;
    return 1;
  }
  else
    return 0;
}

void setup()
{
  // initialize LCD
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(4, 0);
  lcd.print("--");
  lcd.setCursor(15, 0);
  lcd.print("--");
  lcd.setCursor(2, 1);
  lcd.print("Tunggu Sebentar");
  lcd.setCursor(2, 2);
  lcd.print("Inisialisasi...");
  for (int bloklcd = 6; bloklcd <= 14; bloklcd++)
  {
    String labellcddisc;
    lcd.setCursor(bloklcd, 0);
    if (bloklcd == 6)
    {
      labellcddisc = "P";
    }
    if (bloklcd == 7)
    {
      labellcddisc = "u";
    }
    if (bloklcd == 8 || bloklcd == 11 || bloklcd == 15)
    {
      labellcddisc = "r";
    }
    if (bloklcd == 9 || bloklcd == 12)
    {
      labellcddisc = "e";
    }
    if (bloklcd == 10)
    {
      labellcddisc = " ";
    }
    if (bloklcd == 11)
    {
      labellcddisc = "T";
    }
    if (bloklcd == 12)
    {
      labellcddisc = "a";
    }
    if (bloklcd == 13)
    {
      labellcddisc = "n";
    }
    if (bloklcd == 14)
    {
      labellcddisc = "k";
    }
    lcd.print(labellcddisc);
    delay(20);
  }
  delay(2000);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("--Pure Tank--");
  lcd.setCursor(2, 1);
  lcd.print("Menghubungkan ke : ");
  Serial.print("Menghubungkan ke : ");
  lcd.setCursor(4, 2);
  lcd.print(ssid);
  Serial.println(ssid);
  delay(1000);
  lcd.clear();
  while (WiFi.status() != WL_CONNECTED)
  {
    String labellcddisc;
    int bloklcd2 = random(0, 3);
    for (int bloklcd = 0; bloklcd <= 19; bloklcd++)
    {
      lcd.setCursor(bloklcd, bloklcd2);
      lcd.print("MENGHUBUNGKAN");
      delay(40);
      lcd.clear();
      delay(10);
    }
  }

  bot.sendMessage(CHAT_ID, "PureTank Aktif, Gunakan /start command.", "");
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("--Pure Tank--");
  lcd.setCursor(7, 1);
  lcd.print("TERHUBUNG");
  Serial.println("TERHUBUNG");
  lcd.setCursor(9, 2);
  lcd.print("KE");
  lcd.setCursor(4, 2);
  lcd.print(ssid);

  // pinmode setup
  pinMode(btn1, INPUT_PULLUP);
  pinMode(btn2, INPUT_PULLUP);
  pinMode(solebaw_p, OUTPUT);
  pinMode(solesum_p, OUTPUT);
  pinMode(solerum_p, OUTPUT);
  pinMode(sanyo_p, OUTPUT);
  pinMode(Trig1_p, OUTPUT);
  pinMode(Echo1_p, INPUT);
  pinMode(Trig2_p, OUTPUT);
  pinMode(Echo2_p, INPUT);
  digitalWrite(solesum_p, LOW);
  digitalWrite(sanyo_p, HIGH);
  digitalWrite(solerum_p, HIGH);
  digitalWrite(solebaw_p, HIGH);

  Stateku = monitor;
}

void reconnectWiFi()
{
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - pMilRec >= interRec))
  {
    Serial.print(millis());
    Serial.println("Internet Terputus, Menghubungkan Ulang...");
    WiFi.disconnect();
    WiFi.reconnect();
    pMilRec = currentMillis;
  }
}

void SenspH()
{
  float calibration_value = 21.34;
  int phval = 0;
  unsigned long int avgval;
  int buffer_apH[10], temp;
  for (int i = 0; i < 10; i++)
  {
    buffer_apH[i] = analogRead(pH_p);
    delay(30);
  }
  for (int i = 0; i < 9; i++)
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buffer_apH[i] > buffer_apH[j])
      {
        temp = buffer_apH[i];
        buffer_apH[i] = buffer_apH[j];
        buffer_apH[j] = temp;
      }
    }
  }
  avgval = 0;
  for (int i = 2; i < 8; i++)
    avgval += buffer_apH[i];
  float voltpH = (float)avgval * 2.5 / 1024 / 5.2;
  pH = -5.70 * voltpH + calibration_value;
}

void SensUlt()
{
  digitalWrite(Trig1_p, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig1_p, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig1_p, LOW);
  dur = pulseIn(Echo1_p, HIGH);
  dis = (dur / 2) / 29;
  WatLv = TankD - dis;
  if (dis < TankD)
  {
    percentage = (WatLv / TankD) * 100;
  }
  else
  {
    percentage = 0;
  }
}

void SensUltWPC()
{
  digitalWrite(Trig2_p, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig2_p, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig2_p, LOW);
  dur2 = pulseIn(Echo2_p, HIGH);
  dis2 = (dur2 / 2) / 29;
  WatLv2 = TankD2 - dis2;
  if (dis2 < TankD2)
  {
    percentage2 = (WatLv2 / TankD2) * 100;
  }
  else
  {
    percentage2 = 0;
  }
}

float round_to_dp(float in_value, int decimal_place)
{
  float multiplier = powf(10.0f, decimal_place);
  in_value = roundf(in_value * multiplier) / multiplier;
  return in_value;
}

void SensTur()
{
  volt = 0;
  for (int i = 0; i < 800; i++)
  {
    volt += ((float)analogRead(Turbid_p) / 1023) * 5;
  }
  volt = volt / 800;
  volt = round_to_dp(volt, 2);
  volttur = volt;
  if (volttur < 2)
  {
    ntu = 3000;
  }
  else if (volttur > 4)
  {
    ntu = random(120, 250);
  }
  else
  {
    ntu = -1120.4 * sq(volttur) + 5742.3 * volttur - 4353.8;
  }
}

void LCD_monitor()
{
  lcd.clear();
  if (WiFi.status() != WL_CONNECTED)
  {
    lcd.setCursor(14, 0);
    lcd.print("SIGNAL");
    lcd.setCursor(15, 1);
    lcd.print("LOST");
  }
  lcd.setCursor(0, 0);
  lcd.print("pH");
  lcd.setCursor(7, 0);
  lcd.print(":");
  lcd.setCursor(9, 0);
  lcd.print(pH, 1);
  lcd.setCursor(0, 1);
  lcd.print("NTU");
  lcd.setCursor(7, 1);
  lcd.print(":");
  lcd.setCursor(9, 1);
  lcd.print(ntu, 1);
  lcd.setCursor(0, 2);
  lcd.print("Volume");
  lcd.setCursor(7, 2);
  lcd.print(":");
  lcd.setCursor(9, 2);
  lcd.print(percentage, 0);
  lcd.print("%");
  lcd.setCursor(16, 2);
  lcd.print(percentage2, 0);
  lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("Durasi");
  lcd.setCursor(7, 3);
  lcd.print(":");
  lcd.setCursor(9, 3);
  lcd.print(menitan, 0);
  lcd.setCursor(11, 3);
  lcd.print("M");
  lcd.setCursor(13, 3);
  lcd.print(detikan, 0);
  lcd.setCursor(15, 3);
  lcd.print("D");
  delay(500);
}

void LCD_monitorstatement()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Apakah Anda Ingin");
  lcd.setCursor(0, 1);
  lcd.print("Membersihkan Tandon?");
  lcd.setCursor(2, 2);
  lcd.print("(A)Ya     (B)Ga");
  lcd.setCursor(0, 3);
  lcd.print("Durasi");
  lcd.setCursor(7, 3);
  lcd.print(menitan, 0);
  lcd.setCursor(9, 3);
  lcd.print("Men.");
  lcd.setCursor(14, 3);
  lcd.print(detikan, 0);
  lcd.setCursor(16, 3);
  lcd.print("Det.");
}

void LCD_durasireset()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Apakah Anda Ingin");
  lcd.setCursor(1, 1);
  lcd.print("Mengulang Durasi?");
  lcd.setCursor(2, 2);
  lcd.print("(A)Ya     (B)Ga");
  lcd.setCursor(0, 3);
  lcd.print("Durasi");
  lcd.setCursor(7, 3);
  lcd.print(menitan, 0);
  lcd.setCursor(9, 3);
  lcd.print("Men.");
  lcd.setCursor(14, 3);
  lcd.print(detikan, 0);
  lcd.setCursor(16, 3);
  lcd.print("Det.");
  delay(100);
}

float bki(float bki_a, float bki_b, float bki_c, float bki_x)
{
  if (bki_x >= bki_a && bki_x <= bki_b)
  {
    return 1;
  }
  else if (bki_x >= bki_a && bki_x <= bki_c)
  {
    return (bki_c - bki_x) / (bki_c - bki_b);
  }
  else
  {
    return 0;
  }
}

float tr(float tr_a, float tr_b, float tr_c, float tr_d, float tr_x)
{
  if (tr_x >= tr_a && tr_x <= tr_b)
  {
    return (tr_x - tr_a) / (tr_b - tr_a);
  }
  else if (tr_x >= tr_b && tr_x <= tr_c)
  {
    return 1;
  }
  else if (tr_x >= tr_c && tr_x <= tr_d)
  {
    return (tr_d - tr_x) / (tr_d - tr_c);
  }
  else
  {
    return 0;
  }
}

float sgt(float sgt_a, float sgt_b, float sgt_c, float sgt_x)
{
  if (sgt_x >= sgt_a && sgt_x <= sgt_b)
  {
    return (sgt_x - sgt_a) / (sgt_b - sgt_a);
  }
  else if (sgt_x >= sgt_b && sgt_x <= sgt_c)
  {
    return (sgt_c - sgt_x) / (sgt_c - sgt_b);
  }
  else
  {
    return 0;
  }
}

float bka(float bka_a, float bka_b, float bka_c, float bka_x)
{
  if (bka_x >= bka_a && bka_x <= bka_b)
  {
    return (bka_x - bka_a) / (bka_b - bka_a);
  }
  else if (bka_x >= bka_b)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void Fuzzyfikasi()
{
  // Anggota himpunan pH
  asam = bki(0, 6, 7, pH);
  netral = tr(6, 7, 8, 9, pH);
  basa = bka(8, 9, 13, pH);

  // Anggota himpunan kekeruhan
  sjernih = bki(0, 1000, 1500, ntu);
  jernih = tr(1000, 1500, 2000, 2500, ntu);
  keruh = bka(2000, 2500, 3000, ntu);
}

int FuzzyRule()
{
  Fuzzyfikasi();
  wbentar = 35;
  wsedang = 70;
  wlama = 100;

  // Inferensi
  // wbentar
  a1 = min(netral, sjernih);
  a2 = min(netral, jernih);
  // wsedang
  a3 = min(asam, sjernih);
  a4 = min(asam, jernih);
  a5 = min(basa, sjernih);
  a6 = min(basa, jernih);
  // wlama
  a7 = min(asam, keruh);
  a8 = min(netral, keruh);
  a9 = min(basa, keruh);

  r1 = a1 * wbentar;
  r2 = a2 * wbentar;

  r3 = a3 * wsedang;
  r4 = a4 * wsedang;
  r5 = a5 * wsedang;
  r6 = a6 * wsedang;

  r7 = a7 * wlama;
  r8 = a8 * wlama;
  r9 = a9 * wlama;

  // defuzzyfikasi
  waktu = (r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9) / (a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9);

  if (waktu > durasi)
  {
    durasi = waktu;
  }
  if (durasi > wlama)
  {
    durasi = wlama;
  }
  return durasi;
}

void validasitandon()
{
  if (durasi < 50)
  {
    Validasikeadaantandon = "LAYAK";
    Vkt = "Aman untuk digunakan.";
  }
  else if (durasi >= 50 || durasi < 90)
  {
    Validasikeadaantandon = "KURANG LAYAK";
    Vkt = "Kurang aman untuk digunakan. Direkomendasikan untuk pembersihan skala sedang.";
  }
  else if (durasi >= 90)
  {
    Validasikeadaantandon = "TIDAK LAYAK";
    Vkt = "Tidak aman untuk digunakan. Direkomendasikan untuk segera melakukan pembersihan.";
    bot.sendMessage(CHAT_ID, "PERHATIAN! Tandon anda dalam kondisi TIDAK LAYAK. Segera lakukan pembersihan\n\n> /mulaibersih Untuk memulai pembersihan.", "");
  }
}

void durasimeter()
{
  int jaman = 0;
  waktuan = durasi;
  jaman = waktuan / 3600;
  menitan = (waktuan - (3600 * jaman)) / 60;
  detikan = (waktuan - (3600 * jaman)) - (menitan * 60);
}

void pembersihset()
{
  digitalWrite(solesum_p, HIGH);
  digitalWrite(solebaw_p, LOW);
  digitalWrite(solerum_p, LOW);
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Mengosongkan");
  lcd.setCursor(1, 2);
  lcd.print("Tandon");
  bot.sendMessage(CHAT_ID, "Mengosongkan Tandon...", "");
  if (percentage < 20)
  {
    percentage = 0;
    String validasibers = "Sedang membersihkan tandon...\nDurasi Total :";
    validasibers += String(menitan) + " Menit " + String(detikan) + " Detik\n";
    bot.sendMessage(CHAT_ID, "Sedang membersihkan tandon...", "");
    int ibersj = 0;
    int ibersm = 0;
    int ibersd = 0;
    for (int ibers = durasi; ibers >= 0; ibers--)
    {
      if (ibers > 2)
      {
        if (buttonB())
        {
          ibers = 2;
        }
      }
      ibersj = (ibers / 3600);
      ibersm = (ibers - (3600 * ibersj)) / 60;
      ibersd = (ibers - (3600 * ibersj)) - (ibersm * 60);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Proses Membersihkan");
      lcd.setCursor(6, 1);
      lcd.print("Tandon");
      lcd.setCursor(2, 3);
      lcd.print("(B) Henti Darurat");
      lcd.setCursor(7, 2);
      if (ibersm >= 10)
      {
        lcd.print(String(ibersm));
      }
      if (ibersm < 10)
      {
        lcd.print("0" + String(ibersm));
      }
      lcd.setCursor(9, 2);
      lcd.print(":");
      lcd.setCursor(10, 2);
      if (ibersd >= 10)
      {
        lcd.print(String(ibersd));
      }
      if (ibersd < 10)
      {
        lcd.print("0" + String(ibersd));
      }
      digitalWrite(sanyo_p, LOW);
      Serial.print("Durasi : ");
      Serial.print(ibers);
      Serial.print("    ");
      Serial.println(ibersv);
      ibersv = 0;
      delay(512);
      lcd.setCursor(9, 2);
      lcd.print(" ");
      delay(512);
    }
    digitalWrite(sanyo_p, HIGH);
    lcd.setCursor(5, 2);
    lcd.print("Tunggu...");
    bot.sendMessage(CHAT_ID, "Proses Pembersihan Selesai,\nTunggu dalam 10 detik...", "");
    for (int itungg = 10; itungg >= 0; itungg--)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Proses Membersihkan");
      lcd.setCursor(5, 1);
      lcd.print("Tunggu...");
      lcd.setCursor(8, 2);
      if (itungg >= 10)
      {
        lcd.print(String(itungg));
      }
      if (itungg < 10)
      {
        lcd.print("0" + String(itungg));
      }
      delay(1024);
    }
    bot.sendMessage(CHAT_ID, "Proses Selesai.");
    digitalWrite(solebaw_p, HIGH);
    digitalWrite(solesum_p, LOW);
    digitalWrite(solerum_p, HIGH);
    percentage = (WatLv / TankD) * 100;
    durasi = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Proses Membersihkan");
    lcd.setCursor(5, 1);
    lcd.print("Tunggu...");
    lcd.setCursor(1, 2);
    lcd.print("--Proses Selesai--");
    delay(1000);
    Stateku = monitor;
  }
}

void SendDB()
{
  SenspH();
  SensTur();
  SensUlt();
  SensUltWPC();
  FuzzyRule();
}

void handleNewMessages(int numNewMessages)
{
  validasitandon();
  Serial.println("Jenis Pesan");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start")
    {
      String welcome = "Halo, " + from_name + ".\n";
      welcome += "Selamat Datang di bot **PureTank**. Silakan gunakan command dibawah ini :\n\n";
      welcome += "> /cekStatus untuk cek kondisi air saat ini.\n";
      welcome += "> /mulaibersih untuk memulai pembersihan tandon.\n";
      welcome += "> /reset untuk mengulang durasi pembersihan kembali ke awal.\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/cekStatus")
    {
      String StatusTandon = "Keadaan Tandon :\n\n";
      StatusTandon += "pH\t\t: " + String(pH) + "\n";
      StatusTandon += "ntu\t\t: " + String(ntu) + "\n";
      StatusTandon += "Volume Air :\n\t\t\t " + String(percentage) + "%\n";
      StatusTandon += "Volume Tandon Pembersih :\n\t\t\t " + String(percentage2) + "%\n\n";
      StatusTandon += "Durasi\t\t: " + String(menitan) + " menit " + String(detikan) + " detik\n\n";
      StatusTandon += "Kesimpulan\t\t:\n";
      StatusTandon += "Air di tandon anda dalam kondisi " + Validasikeadaantandon + ". " + Vkt;

      bot.sendMessage(chat_id, StatusTandon, "");
    }
    if (text == "/reset")
    {
      durasi = 0;
      delay(1000);
      String resetTandon = "Durasi berhasil direset :\n";
      resetTandon += String(menitan) + " menit ";
      resetTandon += String(detikan) + " detik\n";
      bot.sendMessage(chat_id, resetTandon, "");
    }
    if (text == "/jadwal")
    {
      String j[5];
      String jadwalTandon = "Masukkan Bulan Untuk Penjadwalan :\n";
      bot.sendMessage(chat_id, jadwalTandon, "");
    }
    if (text == "/mulaibersih")
    {
      Stateku = monitorstatement;
      String MembersihkanTandon = "Apakah anda ingin membersihkan Tandon dengan durasi :\n";
      MembersihkanTandon += String(menitan) + " menit ";
      MembersihkanTandon += String(detikan) + " detik ?\n\n";
      MembersihkanTandon += "> /Ya\n";
      MembersihkanTandon += "> /Ndak\n";
      bot.sendMessage(chat_id, MembersihkanTandon, "");
    }
    if (Stateku == monitorstatement)
    {
      if (text == "/Ya")
      {
        Stateku = bersih;
        if (Stateku == monitor)
        {
          bot.sendMessage(chat_id, "Proses pembersihan selesai");
        }
      }
      if (text == "/Ndak")
      {
        Stateku = monitor;
      }
    }
  }
}

void telegramstatus()
{
  if (millis() > lastTimeBotRan + botRequestDelay)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("Mendapatkan Respon Telegram");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}

void loop()
{

  switch (Stateku)
  {
  case monitor:
    Serial.println("Status : Monitor");
    reconnectWiFi();
    SendDB();
    LCD_monitor();
    durasimeter();
    telegramstatus();
    if (buttonB())
    {
      Stateku = monitorstatement;
    }
    if (buttonA())
    {
      Stateku = durasireset;
    }
    break;
  case bersih:
    telegramstatus();
    pembersihset();
    if (buttonB())
    {
      ibersv = 1;
    }
    break;
  case monitorstatement:
    telegramstatus();
    LCD_monitorstatement();
    if (buttonB())
    {
      lcd.clear();
      Stateku = monitor;
    }
    if (buttonA())
    {
      lcd.clear();
      Stateku = bersih;
    }
    break;
  case durasireset:
    LCD_durasireset();
    if (buttonB())
    {
      lcd.clear();
      Stateku = monitor;
    }
    if (buttonA())
    {
      lcd.clear();
      durasi = 0;
      Stateku = monitor;
    }
    break;
  case penjadwalan:
    telegramstatus();
    break;
  default:
    break;
  }
}
