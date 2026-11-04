# Era TV bill of materials

Parts for **five** Era TV gift builds (plus one spare display board for development), using the [CYD Retro MiniTV](https://makerworld.com/en/models/2777502-cyd-retro-minitv) case by DynaMight with a channel knob, two volume buttons, and a rear USB-C power jack with an on/off switch. Assembly steps are in [ERA_TV_BUILD.md](ERA_TV_BUILD.md).

Prices are the AliExpress US (USD) cart prices on 2026-11-04, after AliExpress's own discounts. AliExpress applies its new-shopper price to **one** item per order (currently the toggle switches), so expect a dollar or two of movement at checkout. Sales tax and any import charges are added at checkout and are not included here.

Every line except the display boards ships free via AliExpress *Choice*, with delivery estimates around Nov 13–19. The boards ship standard from the seller ($18.05), estimated Nov 14–21. The estimates were shown for the cart's default Alabama address, so your real dates depend on your address.

## AliExpress cart

| # | Part | Qty in cart | Per TV | Price (USD) | Listing |
|---|------|-------------|--------|-------------|---------|
| 1 | ESP32-2432S028R "CYD" 2.8" display board, **2USB / ST7789**, 3-pack (DIYTZT) | 2 × 3-pack = 6 boards | 1 | 60.98 + 18.05 shipping | [DIYTZT](https://www.aliexpress.us/item/3256808038617309.html) — option *2.8 Inch ST7789 / 3pcs* |
| 2 | 2415 oval speaker, 8 Ω 1 W, 24 × 15 mm, bare 60 mm leads | 1 × 5-pack | 1 | 1.90 | [Eiechip](https://www.aliexpress.us/item/3256809614832360.html) |
| 3 | EC11 rotary encoder with push switch, 20 detents, 20 mm D ("half") shaft | 1 × 5-pack | 1 | 4.09 | [LCdatalink](https://www.aliexpress.com/item/1005008583446552.html) — option *20mm Half Handle* |
| 4 | 6 × 6 × 9 mm tact switch, 4-pin DIP | 20 | 2 | 0.96 | [DQLZV](https://www.aliexpress.com/item/1005005562524949.html) — *6X6X9MM / 20PCS* |
| 5 | JST 1.25 mm (MX1.25) 4-pin plug + socket pairs, 10 cm | 10 pairs | see note | 3.90 | [LOLOIY](https://www.aliexpress.com/item/1005008624362200.html) — *4Pin / 10Pairs* |
| 6 | JST 1.25 mm (MX1.25) 4-pin single-head plug, 20 cm, 28 AWG | 10 | see note | 2.12 | [XMS](https://www.aliexpress.com/item/1005007277110532.html) — *20CM / 4P / Single head* |
| 7 | JST 1.25 mm (MX1.25) 2-pin plug + socket pairs, 10 cm (speaker lead) | 10 pairs | 1 plug | 3.03 | [LOLOIY](https://www.aliexpress.com/item/1005008624362200.html) — *2Pin / 10Pairs* |
| 8 | USB-C panel jack, 2-wire power, black | 10 | 1 | 3.84 | [Ace Wong's](https://www.aliexpress.com/item/1005006297697300.html) — *Black / 10pcs* |
| 9 | MTS-101 mini toggle switch, SPST on/off, 6 mm hole | 10 | 1 | 1.09 | [Electrical store](https://www.aliexpress.com/item/4000032608294.html) |
| 10 | 10 kΩ ¼ W 1% metal film resistor | 100 | 2 | 1.24 | [UltimateElectronics](https://www.aliexpress.us/item/2251832129615028.html) |
| 11 | M3 × 6 mm button-head hex screws (CYD to case) | 50 | 4 | 1.66 | [ZONSANTA](https://www.aliexpress.com/item/1005002357962115.html) — *M3 x 50pcs / 6mm* |
| 12 | M2 × 6 mm countersunk hex screws (back panel) | 50 | 4 | 1.72 | [NINDEJIN](https://www.aliexpress.com/item/4000239264456.html) — *M2 50pcs / 6mm* |
| 13 | Heat-shrink tubing assortment, 164 pcs | 1 | — | 1.24 | [UMLIFE](https://www.aliexpress.com/item/1005006863956051.html) |
| | **Items subtotal** | | | **87.77** | |
| | **Shipping** (display boards only) | | | **18.05** | |
| | **Coupon** ($3 off $15) | | | **−3.00** | |
| | **Estimated total, before tax** | | | **≈ 102.82** | |

Links on `aliexpress.com` redirect to the matching `aliexpress.us` listing.

**Connector note:** each TV uses three 4-pin plugs (board connectors CN1, P3, and P1) and one 2-pin plug (speaker socket). Items 5 and 6 give 20 four-pin plugs for 15 needed (18 with the spare board). Only use the half of each pair whose white housing plugs onto the board's male header. The unused mating halves are handy short extension leads, so no separate hookup wire was ordered.

## Why these parts

- **2USB/ST7789 board.** The CYD Retro MiniTV case was designed around this variant, and the firmware's `BOARD_CYD_ERA_TV` profile uses the same display settings as the RetroTV profile, which already runs this board. All six boards come from one store so their display settings match. Most other listings cap deal-priced boards at one per account, which would have forced a mixed order.
- **Encoder.** 20 detents with one full quadrature cycle per click matches `ENCODER_STEPS_PER_DETENT 4`. The D-shaft makes printed knobs easy, and the push switch replaces the case's original front button.
- **Tact switches.** These are the same 6 × 6 × 9 mm switch the case already uses for its front button, so the volume buttons can copy its friction-fit pocket.
- **USB-C jack and screws.** These are the exact items the case designer specifies, so the back panel and mounting bosses fit.

## Buy locally instead

These were left out on purpose. AliExpress versions are unreliable or a safety concern for gifts, or you probably already have them.

| Item | Qty | Notes |
|------|-----|-------|
| microSD cards, 32 GB, genuine SanDisk or Samsung | 5 | Counterfeit cards are common on AliExpress and fail under sustained video reads. 32 GB is easy to format as FAT32 and holds roughly 25–30 hours of 320 × 240 video. |
| USB-A wall chargers, 5 V ≥ 1 A, UL-listed | 5 | One per recipient. Use a USB-A charger: the panel jack has only two wires, so some USB-C-to-USB-C chargers won't supply power to it. |
| USB-A to USB-C cables, 1 m | 5 | AliExpress limits deal-priced cables to one per account. |
| PLA filament | roughly 50–70 g per TV | Pick colors per era, such as walnut brown for the '70s, silver and black for the '80s, or translucent colors for the '90s. 1.75 mm filament also becomes the rabbit-ear antennas. |
| Thin cyanoacrylate glue, hot glue, double-sided foam tape | — | For the grille, VCR, speaker, and tact-switch pockets. |

## Tools

Soldering iron and solder, flush cutters, wire strippers for 28 AWG, a multimeter with continuity beep, a heat gun or lighter for heat shrink, a 7 mm and a 6 mm drill bit (or a step drill) if you modify printed parts instead of the CAD files, a small hacksaw or rotary tool to shorten encoder shafts, a 2 mm and a 2.5 mm hex key, and a USB data cable for flashing.
