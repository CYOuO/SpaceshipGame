# SpaceshipGame

以 C++、OpenGL 與 GLUT 製作的 3D 太空冒險遊戲。玩家需要在 25 秒內閃避障礙物、收集愛心，並盡可能取得高分。

## 開啟方式

1. 安裝 Visual Studio（含「使用 C++ 的桌面開發」工作負載）。
2. 安裝並啟用 [Git LFS](https://git-lfs.com/)；大型貼圖、模型與音樂素材皆由 Git LFS 管理。
3. 複製本專案並開啟 `tutorial3.sln`。
4. 還原 `packages.config` 中的 NuGet 套件。
5. 建置並執行專案。

## 一、操作說明

| 按鍵 | 功能 |
| --- | --- |
| `空白鍵` | 開始遊戲 |
| `W` | 向上移動 |
| `S` | 向下移動 |
| `A` | 向左移動 |
| `D` | 向右移動 |
| `Z` | 向前移動 |
| `X` | 向後移動 |
| `R` | 遊戲結束時重新開始 |
| `Esc` | 退出遊戲 |

## 二、遊玩規則

### 1. 生存目標

- 在 25 秒內盡可能生存並獲得高分。
- 避開所有障礙物以保持生命值。
- 收集愛心來恢復生命值。

### 2. 生命系統

- 初始生命值：3 點。
- 撞到障礙物：扣除 1 點生命。
- 碰到愛心：恢復 1 點生命。
- 生命值歸零時，遊戲直接結束。

### 3. 計分系統（滿分 100 分）

- 存活獎勵：每存活 1 秒獲得 2 分。
- 碰撞懲罰：撞到小型障礙物扣 1 分，撞到大型障礙物扣 3 分。
- 完成獎勵：
  - 滿血完成：額外獲得 50 分。
  - 剩餘 2 點生命：額外獲得 40 分。
  - 剩餘 1 點生命：額外獲得 30 分。

## 三、素材來源

### 1. Skybox

- [Free 8K Space and Galaxies HDRI－CGTrader](https://www.cgtrader.com/free-3d-models/space/other/free-8k-space-and-galaxies-hdri)

### 2. 外星人模型

- [Cartoon Alien V1.001－CGTrader](https://www.cgtrader.com/free-3d-models/various/various-models/cartoon-alien-v1-001)

### 3. 愛心模型與貼圖

- 3DS 模型：[Simple Symbolic Heart－CGTrader](https://www.cgtrader.com/free-3d-models/various/various-models/simple-symbolic-heart)
- BMP 貼圖：[CGTrader 下載頁面](https://www.cgtrader.com/items/2006541/download-page)

### 4. 背景音樂

- 歌曲：Jim Yosef & Alex Skrindo－Ruby [NCS Release]
- 音樂由 NoCopyrightSounds 提供。
- [免費下載／串流](http://ncs.io/Ruby)
- [YouTube 觀看](http://youtu.be/Np-Y8ClGgRk)
