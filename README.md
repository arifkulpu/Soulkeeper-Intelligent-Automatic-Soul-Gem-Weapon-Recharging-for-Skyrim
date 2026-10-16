# Soulkeeper — Intelligent & Automatic Soul Gem Weapon Recharging for Skyrim

![Skyrim Special Edition & Anniversary Edition](https://img.shields.io/badge/Skyrim-SE%20%7C%20AE%20%7C%20VR-blue.svg)
![Platform](https://img.shields.io/badge/Platform-SKSE64%20%2F%20CommonLibSSE--NG-green.svg)
![License](https://img.shields.io/badge/License-Proprietary-red.svg)

[Türkçe](#türkçe-açıklama) | [English](#english-description)

---

## 📋 Güncelleme Günlüğü / Changelog

### v1.2 — Canlı Şarj Takibi Genişletmesi *(2026-09-12)*

**Türkçe:**
- **Yakın dövüş silahları** (hançer, kılıç, balta, topuz vb.) artık her vuruşta şarj miktarını **anında** düşürür. Skyrim motoru bu silahlar için şarjı yalnızca envanter açılınca güncellediğinden, `TESHitEvent` ile her vuruş yakalanıp şarj kendi kendine düşürülür.
- **Yaylar** kirişi her bıraktığınızda (`TESPlayerBowShotEvent`) şarjı anında günceller; yakın dövüşle aynı yöntemle takip edilir.
- **Arbaletler** her cıvata vuruşunda (`TESHitEvent`) anında güncellenir; kaynak olarak arbelet silah FormID'si kullanılır.
- **Asalar** her büyü vuruşunda (`TESHitEvent`) anında güncellenir; motor, kaynak olarak silah FormID'si yerine enchantment FormID'sini gönderdiğinden eşleştirme buna göre yapılır.
- Artık tüm silah türlerinde güncel şarjı görmek için **envanter açmaya gerek yok**.

**English:**
- **Melee weapons** (daggers, swords, axes, maces, etc.) now instantly reflect charge loss on every hit. Since the engine only flushes `ExtraCharge` on inventory open, each strike is caught via `TESHitEvent` and charge is deducted proactively.
- **Bows** now update charge the moment you release the string (`TESPlayerBowShotEvent`), using the same approach as melee.
- **Crossbows** update charge on every bolt impact (`TESHitEvent`), matched by crossbow weapon FormID in the event source field.
- **Staves** update charge on every spell impact (`TESHitEvent`), matched by enchantment FormID — the engine sends the spell/enchantment as the source, not the staff weapon itself.
- **No need to open inventory** to see the current charge of any weapon type.

---

<a name=english-description></a>
## 🇬🇧 English Description

**Soulkeeper** is a high-performance, native (C++ / SKSE64) automatic weapon recharging and passive enchantment regeneration mod for Skyrim Special Edition and Anniversary Edition. Follower Unbound manages follower soul gem purchasing and stocking.

Never worry about your enchanted weapons running out of juice in the middle of combat, clunky inventory soul gem menus, or followers equipped with depleted weapons. Soulkeeper handles everything seamlessly in real-time.

---

### 🌟 Key Features

#### 1. ⚔️ Smart Auto-Recharge (Player & Followers)
- **Optimal Soul Gem Selection**: Automatically picks the best-fitting soul gem (Petty, Lesser, Common, Greater, Grand, Black) depending on missing weapon charge to minimize wasted capacity.
- **Target Charge Cap (%)**: Set a target ceiling (Default: **50%**) so valuable high-tier soul gems are not wasted on minor top-offs.
- **Seamless Combat Experience**: Instant native charge updates using **Actor::UpdateWeaponAbility** and **InventoryChanges** without unequipping, sheathing, or interrupting attack animations.
- **HUD & UI Synchronization**: Full real-time synchronization with Skyrim's native UI, SkyUI, iEquip, TrueHUD, and Equipment HUD bars.

#### 2. 🛡️ Follower Soul Gem Management
- Follower Unbound manages follower soul gem purchases and stocking.
- Soulkeeper recharges follower weapons from soul gems already in their inventory and never purchases or restocks gems.

#### 3. ⏳ Passive Enchantment Regeneration (Passive Recharge)
- **Natural Over-Time Recovery**: Equipped enchanted weapons naturally regain **1%** charge every 60 seconds of real gameplay without consuming any soul gems.
- **Safety Emergency Cap (Default: 20%)**: Passive recharge stops once weapon charge reaches the customizable cap (**20%**), acting as an emergency charge buffer rather than an unlimited free recharge cheat.
- **Full Time-Skip Integration**: Seamlessly calculates elapsed In-Game Time (IGT) when sleeping in taverns, waiting, or fast-traveling across the map—instantly refilling weapons up to the cap limit upon arrival.
- **Zero Notification Clutter**: Works completely silently in the background when weapons are already above the cap limit.
- **Follower Support**: Active followers benefit from the same passive enchantment recovery on their equipped weapons.

#### 4. 🎛️ Live In-Game SKSE Menu (ImGui via SKSE Menu Framework)
- Open the dedicated configuration & diagnostics overlay anytime in-game by pressing **F1**:
  - **General Settings Tab**: Charge threshold, target recharge cap, notification toggles, and charge values.
  - **Live Status & Controls Tab**: Inspection of equipped weapons, charge values, color-coded progress bars, and follower gold tracker.

---

<a name=türkçe-açıklama></a>
## 🇹🇷 Türkçe Açıklama

**Soulkeeper**, Skyrim Special Edition ve Anniversary Edition için geliştirilmiş, yüksek performanslı ve tamamen yerel (C++ / SKSE64) bir otomatik silah şarj etme ve pasif büyü yenileme modudur. Yoldaşların ruh taşı satın alma ve stoklama işlerini Follower Unbound yönetir.

---

### 🌟 Temel Özellikler

#### 1. ⚔️ Akıllı Otomatik Şarj (Oyuncu & Yoldaşlar)
- **Akıllı Ruh Taşı Seçimi**: Silahın eksik şarj miktarına göre en uygun boyuttaki ruh taşını otomatik seçip kullanır.
- **Hedef Şarj Sınırı (Target Cap %)**: Yüksek seviyeli taşların küçük eksiklikler için boşa gitmesini engellemek adına hedef şarj sınırı belirlenebilir.
- **Savaş Esnasında Kesintisiz Kullanım**: Silahı kınına sokmadan veya animasyonları kesmeden **Actor::UpdateWeaponAbility** ve **InventoryChanges** ile doğrudan günceller.
- **HUD & SkyUI / iEquip Senkronizasyonu**: iEquip, TrueHUD ve SkyUI barlarıyla anında senkronize olur.

#### 2. 🛡️ Yoldaş Ruh Taşı Yönetimi
- Yoldaşların ruh taşı satın alma ve stoklama işlerini Follower Unbound yönetir.
- Soulkeeper, yoldaşın envanterindeki taşlarla silahlarını şarj eder; otomatik satın alma veya stok yenileme yapmaz.

#### 3. ⏳ Pasif Büyü Yenileme (Passive Recharge)
- **Zamanla Kendiliğinden Dolum**: Büyülü silahlarınız hiçbir ruh taşı harcamadan, normal oyun esnasında her 60 saniyede bir azami şarjının **%1'ini** yavaşça geri kazanır.
- **Acil Durum Tavan Sınırı (Varsayılan: %20)**: Pasif şarj silahı sınırsız şekilde %100 doldurmaz; silahın tamamen sönüp etkisiz kalmasını önlemek için **%20 seviyesine kadar** (acil durum şarjı olarak) yeniler. Silahınız zaten bu sınırın üzerindeyse sessizce bekler.
- **Zaman Atlamaları (Uyuma / Bekleme / Hızlı Seyahat)**: Handa uyuduğunuzda veya haritada hızlı seyahat yaptığınızda geçen oyun süresi anında hesaplanır ve silahlarınız vardığınız anda %20 sınırına kadar yenilenmiş olur.
- **Yoldaşlar İçin de Geçerlidir**: Takipçilerinizin ellerindeki büyülü silahlar da aynı şekilde pasif olarak yenilenir.

#### 4. 🎛️ Canlı SKSE In-Game MCM Menüsü (ImGui / SKSE Menu Framework)
- Oyun içerisindeyken **F1** tuşuna basarak açılan özel ImGui arayüzü:
  - **General Settings**: Eşik değerleri, hedef sınırları, şarj miktarları ve bildirim ayarları.
  - **Live Status & Controls**: Kuşanılmış silahların şarj durumu, doluluk oranları, renkli ilerleme barları ve yoldaş altın/silah bilgileri.

---

### 💎 Soul Gem Charge Values
 
Soul gem recharge values (points restored) are configurable in the `F1` menu and `Soulkeeper.ini`. Follower Unbound manages purchase prices and stocking.

| Soul Gem Level | Contained Soul | Default Charge |
| :--- | :---: | :---: |
| **Petty** | Petty | **250 pts** |
| **Lesser** | Lesser | **500 pts** |
| **Common** | Common | **1000 pts** |
| **Greater** | Greater | **2000 pts** |
| **Grand / Black** | Grand | **3000 pts** |

---

### ⚙️ Configuration (`Soulkeeper.ini`)

Settings can be customized live in-game via the **`F1`** menu or edited directly in `Data/SKSE/Plugins/Soulkeeper.ini`:

| Setting | Default | Description |
| :--- | :--- | :--- |
| `bEnablePlayerAutoCharge` | `true` | Enables automatic soul gem recharging for the player. |
| `bEnableFollowerAutoCharge`| `true` | Enables automatic recharging for active followers. |
| `uChargePetty` | `250` | Recharge amount provided by Petty Soul Gem. |
| `uChargeLesser` | `500` | Recharge amount provided by Lesser Soul Gem. |
| `uChargeCommon` | `1000` | Recharge amount provided by Common Soul Gem. |
| `uChargeGreater` | `2000` | Recharge amount provided by Greater Soul Gem. |
| `uChargeGrand` | `3000` | Recharge amount provided by Grand / Black Soul Gem. |
| `fChargeThreshold` | `20.0` | Recharging triggers when weapon charge drops below this percentage (%). |
| `fAutoChargeTargetPercent` | `50.0` | Maximum charge ceiling for automatic soul gem charging (%). |
| `bEnablePassiveRecharge` | `true` | Enables slow passive recharge over in-game time. |

---

### 🔄 Uyumluluk / Compatibility

- **SkyUI & SkyUI Widgets**: %100 Uyumlu.
- **iEquip / Equipment HUD / TrueHUD**: %100 Uyumlu.
- **Quick Inventory & Özel Menü Modları**: %100 Uyumlu.
- **Modlarla Eklenen Tüm Özel Büyülü Silahlar**: %100 Uyumlu.

---

### 📋 Gereksinimler

- **Skyrim Special Edition / Anniversary Edition** (1.5.97, 1.6.353, 1.6.640, 1.6.1130, 1.6.1170, 1.7.104+) veya **Skyrim VR** (1.4.15)
- **SKSE64** (Skyrim Script Extender)
- **SKSE Menu Framework** (`F1` ImGui menüsü için gereklidir)
- **Address Library for SKSE Plugins**

---

## 📜 License / Lisans

Copyright (c) 2026 Arif KULPU. All Rights Reserved. — Tüm Hakları Saklıdır. See [LICENSE.md](LICENSE.md) for details.
