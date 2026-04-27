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

## Mérésekhez használt hardware

**Processzor:**	11th Gen Intel(R) Core(TM) i5-11320H @ 3.20GHz   3.19 GHz
**Memória mérete:**	32,0 GB
**Videokártya:** NVIDIA GeForce RTX 3050 Ti Laptop GPU (4 GB), Intel(R) Iris(R) Xe Graphics (128 MB)


## Mérések
A párhuzamosítás hatékonyságának igazolására a program automatizált teljesítményméréseket végez, amelyek során a futási időket és a számítási képességet (MCUPS - Million Cells Updated Per Second) vizsgálja. Az eredményekről SVG grafikonok készülnek.
A mellékelt grafikonok és adatsorok alapján a szekvenciális (CPU) és a párhuzamos (GPU) végrehajtás összehasonlításakor a következő architekturális és rendszerszintű jelenségek figyelhetők meg:

**1. Rácsméret skálázódása (1000x1000 - 2000x2000)**

A CPU teljesítménye a terhelés növekedésével végig stabil, a szekvenciális végrehajtás áteresztőképessége fixen 203-206 MCUPS között marad. A GPU ezzel szemben egy dinamikus, V-alakú teljesítménygörbét ír le.

* **(1000x1000 - 1200x1200):** A teszt elején a GPU azonnal eléri a maximális potenciálját. 1000x1000-es méretnél a teljesítmény 18 239 MCUPS, ami 88.4-szeres gyorsulást jelent a CPU-hoz képest. A kártya ebben a fázisban "hideg", így maximális órajelen képes üzemelni.

* **(1400x1400 - 1600x1600):** 1400-as rácsméretnél egy drasztikus letörés látható a grafikonon: a teljesítmény hirtelen 3565 MCUPS-ra, a gyorsulás pedig 17.5-szeresre esik vissza. Ennek két oka lehet. Egyrészt az adathalmaz mérete itt éri el azt a kritikus pontot, ahol a videókártya memóriája telítődik a szomszédok egyidejű olvasása miatt. Másrészt a folyamatos maximális terhelés miatt a GPU eléri a fogyasztási vagy hőmérsékleti limitjét, és a driver biztonsági okokból visszaveszi az órajeleket.

* **(1800x1800 - 2000x2000):** Ahogy a rács tovább növekszik, a GPU az alacsonyabb energiaállapot (leszabályozott órajel) ellenére is egyre hatékonyabban tudja beosztani a több millió szálat. A grafikon újra emelkedni kezd, és 2000x2000-es méretnél a teljesítmény visszakapaszkodik 8268 MCUPS-ra (40.2x gyorsulás), bizonyítva, hogy a masszív adatpárhuzamosság még korlátozott hardveres frekvencián is felülmúlja a CPU-t.

**2. Iterációszám skálázódása (Fix 1024x1024 rács)**

Az iterációszám növelése klasszikus esetben egy egyenletesen telítődő görbét eredményezne, azonban a mérési adatok itt is az operációs rendszer és a GPU beavatkozását mutatják.

* **(10 - 100 iteráció):** Alacsony iterációszámnál (10) a gyorsulás 40.9x, mivel a futási idő jelentős részét még a PCIe adatátvitel és a kernel inicializálása (overhead) teszi ki. 100 iterációnál érjük el az ideális egyensúlyt: az indítási költség elaprózódik, a GPU még nem melegszik túl, így a teljesítmény egy hatalmas tüskével eléri a 16 758 MCUPS-ot (a csúcsot jelentő 90.6-szoros gyorsulást).

* **(500 - 2000 iteráció):** 500 iterációnál a teljesítmény ismét beszakad (2867 MCUPS, 13.7x gyorsulás). Ez a jelenség a TDR (Timeout Detection and Recovery) mechanizmusra és az ütemezőre vezethető vissza. Ha az operációs rendszer azt érzékeli, hogy egyetlen kernel túl sokáig (több tizedmásodpercig) tartja 100%-osan lefoglalva a GPU-t megszakítás nélkül, visszaveszi a prioritását, kontextust vált, vagy leszabályozza a kártyát, hogy a grafikus felület ne fagyjon le. Az iterációszám további növelésével (1000, 2000) a hasznos számítási idő aránya ismét javul, így a görbe megkezdi a visszakapaszkodást, elérve a 6239 MCUPS-ot a teszt végére.

Végső konklúzió: A generált grafikonok és mérések egyértelműen igazolják a GPU-alapú OpenCL párhuzamosítás nyers erejét, amely ideális "rövid, de intenzív" Burst terhelések esetén akár 90-szeres gyorsulást is képes produkálni a CPU-hoz képest. Ugyanakkor az adatok vizuális letörései tökéletesen rávilágítanak a GPGPU programozás legnagyobb valós kihívásaira: a tartós csúcsteljesítményt nem csupán az algoritmus hatékonysága, hanem a memóriasávszélesség korlátai, a kártya termikus kerete (Thermal Throttling) és az operációs rendszer biztonsági mechanizmusai is drasztikusan befolyásolják.

