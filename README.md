# ParhuzamosEszkozok_2026
# OpenCL (Game of Life) Szimuláció

## Projekt leírása
Ez a projekt a Conway-féle Game of Life implementációja, amelynek fő célja a hagyományos, szekvenciális CPU-alapú végrehajtás és a masszívan párhuzamosított, OpenCL alapú GPU végrehajtás teljesítményének összehasonlítása. 

A szoftver adatpárhuzamos megközelítést alkalmaz a SIMT (Single Instruction, Multiple Threads) architektúra kihasználásával. A memóriahozzáférések optimalizálása és a versenyhelyzetek elkerülése érdekében a szimuláció a Ping-Pong Buffer technikát alkalmazza a videókártya memóriájában (VRAM). A program egyedi, natív C nyelven írt SVG generátort is tartalmaz a mérési eredmények vizualizálására.

## Rendszerkövetelmények
A szoftver lefordításához és futtatásához az alábbi környezet és könyvtárak szükségesek:
* **Operációs rendszer:** Windows (MinGW környezetben) vagy Linux
* **Fordító:** GCC (GNU Compiler Collection)
* **Párhuzamosítási API:** OpenCL SDK (Intel, NVIDIA vagy AMD driver részeként telepítve)
* **Grafikus könyvtár:** SDL2 (Simple DirectMedia Layer) a valós idejű megjelenítéshez
* **Építőeszköz:** Make

## Mappaszerkezet

* **kernels/**: Az OpenCL kernel forráskódokat tároló mappa (`life.cl`).
* **include/**: A C fejlécfájlok (`.h`) mappája, amelyek a modulok interfészeit definiálják.
* **src/**: A forráskódok (`.c`) mappája.
  * `main.c`: A fő vezérlő logika, a mérési rutinok és az SVG grafikon generátor.
  * `ocl_utils.c`: Az OpenCL kontextus, a parancssor (Command Queue) és a memóriabufferek kezelése.
  * `sdl_utils.c`: Az SDL2 grafikus ablak inicializálása és a textúrák frissítése.
  * `kernel_loader.c`: A külső OpenCL kernel fájlok beolvasásáért felelős modul.
* **Makefile**: A projekt automatikus fordítását végző szkript.
* **.gitignore**: A verziókövetésből kizárandó fájlok listája.

## Futtatás
A szoftver parancssorból fordítható a Make segédprogram segítségével. A tiszta fordítás érdekében érdemes a korábbi objektumfájlokat törölni:
```bash
    make clean
    make
```
A sikeres fordítás után a generált futtatható állomány indítása:
```bash
    ./gol_opencl.exe
```
**Működési módok:**
A program kétféle üzemmódban képes futni, amelyet a `src/main.c` fájlban található `benchmark_mode` logikai változóval lehet szabályozni:
* `benchmark_mode = true`: Automatikus mérési mód. A program lefut grafikus felület nélkül különböző rácsméreteken és iterációszámokon, a konzolba printeli az eredményeket, majd legenerálja az SVG grafikonokat.
* `benchmark_mode = false`: Grafikus mód. A program megnyit egy ablakot, és valós időben vizualizálja a sejtautomata működését.

## Az OpenCL Párhuzamosítás Anatómiája
A párhuzamosítás a CPU (Host) és a GPU (Device) összehangolt munkájának eredménye.

**1. A munka kiosztása (Host oldal)**
A C kódban a feladat kiosztása aszinkron módon történik. A CPU definiál egy kétdimenziós rácsot (NDRange), amely összesen szélesség * magasság darab önálló szálat (Work-Item) jelent. A CPU nem várja meg a számítás végét, csupán beállítja a paramétereket a parancssorba (Command Queue), és utasítja a GPU-t a kernel kód tömeges végrehajtására.

**2. A munka elvégzése (Device oldal)**
A GPU-n futó kernel kódban nincsenek rácson végigmenő ciklusok. A hardver több ezer magja a SIMT elv alapján egyszerre futtatja le ugyanazt a kódblokkot. A `get_global_id()` függvények segítségével minden egyes futó szál lekérdezi a saját pozícióját a rácsban, és kizárólag a hozzá tartozó memóriacellán végzi el a szomszédság vizsgálatát és az állapotfrissítést.

## Mérések
A párhuzamosítás hatékonyságának igazolására a program automatizált teljesítményméréseket végez, amelyek során a futási időket és a számítási képességet (MCUPS - Million Cells Updated Per Second) vizsgálja. Az eredményekről SVG grafikonok készülnek.