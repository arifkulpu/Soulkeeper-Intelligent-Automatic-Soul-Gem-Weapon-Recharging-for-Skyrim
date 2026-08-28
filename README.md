# Soulkeeper — Intelligent & Automatic Soul Gem Weapon Recharging for Skyrim

![Skyrim Special Edition & Anniversary Edition](https://img.shields.io/badge/Skyrim-SE%20%7C%20AE%20%7C%20VR-blue.svg)
![Platform](https://img.shields.io/badge/Platform-SKSE64%20%2F%20CommonLibSSE--NG-green.svg)
![License](https://img.shields.io/badge/License-Proprietary-red.svg)

[Türkçe](#türkçe-açıklama) | [English](#english-description)

---

<a name=english-description></a>
## 🇬🇧 English Description

**Soulkeeper** is a high-performance, native (C++ / SKSE64) automatic weapon recharging, follower soul gem stocking, and passive enchantment regeneration mod for Skyrim Special Edition and Anniversary Edition.

Never worry about your enchanted weapons running out of juice in the middle of combat, clunky inventory soul gem menus, or followers equipped with depleted weapons. Soulkeeper handles everything seamlessly in real-time.

---

### 🌟 Key Features

#### 1. ⚔️ Smart Auto-Recharge (Player & Followers)
- **Optimal Soul Gem Selection**: Automatically picks the best-fitting soul gem (Petty, Lesser, Common, Greater, Grand, Black) depending on missing weapon charge to minimize wasted capacity.
- **Target Charge Cap (%)**: Set a target ceiling (Default: **50%**) so valuable high-tier soul gems are not wasted on minor top-offs.
- **Seamless Combat Experience**: Instant native charge updates using **Actor::UpdateWeaponAbility** and **InventoryChanges** without unequipping, sheathing, or interrupting attack animations.
- **HUD & UI Synchronization**: Full real-time synchronization with Skyrim's native UI, SkyUI, iEquip, TrueHUD, and Equipment HUD bars.

#### 2. 🛡️ Follower Town Stocking & Immersive Vendor Procurement
- **Pre-Expedition Soul Gem Stocking**:
  - Configure desired target stock quantities per tier (e.g. 5 Petty, 3 Lesser, 2 Common, etc.) in the MCM menu.
  - Whenever you enter a town, city, shop, or tavern near a wizard/vendor, followers automatically restock up to your target counts using their carried gold.
  - S-Tier preparedness: Followers carry plenty of gems before entering dungeons, so they don't need to visit towns after every small skirmish!
- **Realistic Location & Merchant Proximity**:
  - Followers will **ONLY** buy soul gems when in a **civilized settlement** and physically close (~50m) to a living **magic vendor, court wizard, or apothecary**.
  - In dungeons and wilderness, followers strictly use the gems already stocked in their inventory.

#### 3. ⏳ Passive Enchantment Regeneration (Passive Recharge)
- **Natural Over-Time Recovery**: Equipped enchanted weapons naturally regain **1%** charge every 60 seconds of real gameplay without consuming any soul gems.
- **Safety Emergency Cap (Default: 20%)**: Passive recharge stops once weapon charge reaches the customizable cap (**20%**), acting as an emergency charge buffer rather than an unlimited free recharge cheat.
- **Full Time-Skip Integration**: Seamlessly calculates elapsed In-Game Time (IGT) when sleeping in taverns, waiting, or fast-traveling across the map—instantly refilling weapons up to the cap limit upon arrival.
- **Zero Notification Clutter**: Works completely silently in the background when weapons are already above the cap limit.
- **Follower Support**: Active followers benefit from the same passive enchantment recovery on their equipped weapons.

#### 4. 🎛️ Live In-Game SKSE Menu (ImGui via SKSE Menu Framework)
- Open the dedicated configuration & diagnostics overlay anytime in-game by pressing **F1**:
  - **General Settings Tab**: Charge threshold, target recharge cap, follower stock quotas per tier, notification toggles, and price caps.
  - **Live Status & Controls Tab**: Inspection of equipped weapons, charge values, color-coded progress bars, and follower gold tracker.

---

<a name=türkçe-açıklama></a>
## 🇹🇷 Türkçe Açıklama

**Soulkeeper**, Skyrim Special Edition ve Anniversary Edition için geliştirilmiş, yüksek performanslı ve tamamen yerel (C++ / SKSE64) bir otomatik silah şarj etme, yoldaş ruh taşı stoklama ve pasif büyü yenileme modudur.

---

### 🌟 Temel Özellikler

#### 1. ⚔️ Akıllı Otomatik Şarj (Oyuncu & Yoldaşlar)
- **Akıllı Ruh Taşı Seçimi**: Silahın eksik şarj miktarına göre en uygun boyuttaki ruh taşını otomatik seçip kullanır.
- **Hedef Şarj Sınırı (Target Cap %)**: Yüksek seviyeli taşların küçük eksiklikler için boşa gitmesini engellemek adına hedef şarj sınırı belirlenebilir.
- **Savaş Esnasında Kesintisiz Kullanım**: Silahı kınına sokmadan veya animasyonları kesmeden **Actor::UpdateWeaponAbility** ve **InventoryChanges** ile doğrudan günceller.
- **HUD & SkyUI / iEquip Senkronizasyonu**: iEquip, TrueHUD ve SkyUI barlarıyla anında senkronize olur.

#### 2. 🛡️ Yoldaş Ruh Taşı Stoklama & Kasabada Otomatik Tedarik
- **Sefer Öncesi Ruh Taşı Stoklama (Stock System)**:
  - F1 menüsünden 5 farklı ruh taşı türü için hedef stok miktarları belirlenebilir (Örn: 5 Petty, 3 Lesser, 2 Common vb.).
  - Bir kasabaya, şehre veya hana gidip yakınınızda bir saray büyücüsü/simyacı olduğunda; yoldaşınız eksik taşları kendi altınıyla topluca satın alıp **çantasına stoklar**.
  - Böylece zindana veya sefere çıkmadan önce yoldaşınızın çantası dolu olur; her savaş sonrası kasaba kasaba taş aramakla uğraşmazsınız!
- **Gerçekçi & Sürükleyici Şehir/Satıcı Kontrolü**:
  - Yoldaşlar **sadece** yerleşim yerlerindeyken ve yakınlarında (~50m) canlı bir **saray büyücüsü, simyacı veya tüccar** varken alışveriş yapabilir.

#### 3. ⏳ Pasif Büyü Yenileme (Passive Recharge)
- **Zamanla Kendiliğinden Dolum**: Büyülü silahlarınız hiçbir ruh taşı harcamadan, normal oyun esnasında her 60 saniyede bir azami şarjının **%1'ini** yavaşça geri kazanır.
- **Acil Durum Tavan Sınırı (Varsayılan: %20)**: Pasif şarj silahı sınırsız şekilde %100 doldurmaz; silahın tamamen sönüp etkisiz kalmasını önlemek için **%20 seviyesine kadar** (acil durum şarjı olarak) yeniler. Silahınız zaten bu sınırın üzerindeyse sessizce bekler.
- **Zaman Atlamaları (Uyuma / Bekleme / Hızlı Seyahat)**: Handa uyuduğunuzda veya haritada hızlı seyahat yaptığınızda geçen oyun süresi anında hesaplanır ve silahlarınız vardığınız anda %20 sınırına kadar yenilenmiş olur.
- **Yoldaşlar İçin de Geçerlidir**: Takipçilerinizin ellerindeki büyülü silahlar da aynı şekilde pasif olarak yenilenir.

#### 4. 🎛️ Canlı SKSE In-Game MCM Menüsü (ImGui / SKSE Menu Framework)
- Oyun içerisindeyken **F1** tuşuna basarak açılan özel ImGui arayüzü:
  - **General Settings**: Eşik değerleri, hedef sınırları, 5 farklı seviye için stok hedefleri, **3 dakikalık hatırlatma periyoduna sahip bildirimler** ve fiyat ayarları.
  - **Live Status & Controls**: Kuşanılmış silahların şarj durumu, doluluk oranları, renkli ilerleme barları ve yoldaş altın/silah bilgileri.

---

### 💰 Standard Soul Gem Price Reference

When followers purchase soul gems in town, standard fair market prices apply:

| Soul Gem Level | Contained Soul | Charge Provided | Gold Cost |
| :--- | :---: | :---: | :---: |
| **Petty** | Petty | **250** | **50 Gold** |
| **Lesser** | Lesser | **500** | **100 Gold** |
| **Common** | Common | **1000** | **150 Gold** |
| **Greater** | Greater | **2000** | **300 Gold** |
| **Grand** | Grand | **3000** | **500 Gold** |

---

### ⚙️ Configuration (`Soulkeeper.ini`)

Settings can be customized live in-game via the **`F1`** menu or edited directly in `Data/SKSE/Plugins/Soulkeeper.ini`:

| Setting | Default | Description |
| :--- | :--- | :--- |
| `bEnablePlayerAutoCharge` | `true` | Enables automatic soul gem recharging for the player. |
| `bEnableFollowerAutoCharge`| `true` | Enables automatic recharging for active followers. |
| `bEnableFollowerStocking` | `true` | Enables automatic pre-stocking of soul gems when in town near a vendor. |
| `uStockPettyCount` | `5` | Desired target stock of Petty Soul Gems in follower inventory (0-20). |
| `uStockLesserCount` | `3` | Desired target stock of Lesser Soul Gems in follower inventory (0-20). |
| `uStockCommonCount` | `2` | Desired target stock of Common Soul Gems in follower inventory (0-20). |
| `uStockGreaterCount` | `0` | Desired target stock of Greater Soul Gems in follower inventory (0-20). |
| `uStockGrandCount` | `0` | Desired target stock of Grand Soul Gems in follower inventory (0-20). |
| `fChargeThreshold` | `99.0` | Recharging triggers when weapon charge drops below this percentage (%). |
| `fAutoChargeTargetPercent` | `50.0` | Maximum charge ceiling for automatic soul gem charging (%). |
| `bAllowFollowerPurchases` | `true` | Allows followers to buy soul gems when in town near a magic vendor. |
| `bEnablePassiveRecharge` | `true` | Enables slow passive recharge over in-game time. |

---

### 🔄 Uyumluluk / Compatibility

- **SkyUI & SkyUI Widgets**: %100 Uyumlu.
- **iEquip / Equipment HUD / TrueHUD**: %100 Uyumlu.
- **Quick Inventory & Özel Menü Modları**: %100 Uyumlu.
- **Modlarla Eklenen Tüm Özel Büyülü Silahlar**: %100 Uyumlu.

---

### 📋 Gereksinimler

- **Skyrim Special Edition** (1.5.97) veya **Skyrim Anniversary Edition** (1.6.640, 1.6.1130, 1.6.1170+)
- **SKSE64** (Skyrim Script Extender)
- **SKSE Menu Framework** (`F1` ImGui menüsü için gereklidir)
- **Address Library for SKSE Plugins**

---

## 📜 License / Lisans

Copyright (c) 2026 Arif KULPU. All Rights Reserved. — Tüm Hakları Saklıdır. See [LICENSE.md](LICENSE.md) for details.
