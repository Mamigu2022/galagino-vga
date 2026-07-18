#ifndef _MACHINES_H_
#define _MACHINES_H_

// disable e.g. if roms are missing
#define ENABLE_PACMAN
#define ENABLE_GALAGA
//#define ENABLE_DKONG
//#define ENABLE_FROGGER
//#define ENABLE_DIGDUG
#define ENABLE_1942
//#define ENABLE_EYES
//#define ENABLE_MRTNT
//#define ENABLE_LIZWIZ
//#define ENABLE_THEGLOB
//#define ENABLE_CRUSH
//#define ENABLE_ANTEATER
#define ENABLE_BOMBJACK
//#define ENABLE_MRDO
#define ENABLE_BAGMAN
//#define ENABLE_PENGO
//#define ENABLE_GYRUSS
#define ENABLE_STARFORCE
#define ENABLE_MOONCRESTA
#define ENABLE_MSPACMAN
#define ENABLE_GALAXIAN
#define ENABLE_SPACE
// #define ENABLE_TIMEPLT
//#define ENABLE_TUTANKHM
//#define ENABLE_LADYBUG
//#define ENABLE_DKONGJR
#define ENABLE_MOONCRESTA
//#define ENABLE_SCRAMBLE
//#define ENABLE_SUPERCOBRA
#ifdef ENABLE_PACMAN  
  #include "machines/pacman/pacman.h"
#endif

#ifdef ENABLE_GALAGA
  #include "machines/galaga/galaga.h"
#endif

#ifdef ENABLE_DKONG
  #include "machines/dkong/dkong.h"
#endif

#ifdef ENABLE_FROGGER
  #include "machines/frogger/frogger.h"
#endif

#ifdef ENABLE_DIGDUG
  #include "machines/digdug/digdug.h"
#endif

#ifdef ENABLE_1942
  #include "machines/1942/1942.h"
#endif

#ifdef ENABLE_EYES
  #include "machines/eyes/eyes.h"
#endif

#ifdef ENABLE_MRTNT
  #include "machines/mrtnt/mrtnt.h"
#endif

#ifdef ENABLE_LIZWIZ
  #include "machines/lizwiz/lizwiz.h"
#endif

#ifdef ENABLE_THEGLOB
  #include "machines/theglob/theglob.h"
#endif

#ifdef ENABLE_CRUSH 
  #include "machines/crush/crush.h"
#endif

#ifdef ENABLE_ANTEATER 
  #include "machines/anteater/anteater.h"
#endif

#ifdef ENABLE_BOMBJACK 
  #include "machines/bombjack/bombjack.h"
#endif

#ifdef ENABLE_MRDO 
  #include "machines/mrdo/mrdo.h"
#endif

#ifdef ENABLE_BAGMAN 
  #include "machines/bagman/bagman.h"
#endif

#ifdef ENABLE_PENGO 
  #include "machines/pengo/pengo.h"
#endif

#ifdef ENABLE_GYRUSS
  #include "machines/gyruss/gyruss.h"
#endif

#ifdef ENABLE_LADYBUG  
  #include "machines/ladybug/ladybug.h"
#endif

#ifdef ENABLE_DKONGJR
  #include "machines/dkongjr/dkongjr.h"
#endif

#ifdef ENABLE_MSPACMAN  
  #include "machines/mspacman/mspacman.h"
#endif

#ifdef ENABLE_TIMEPLT 
  #include "machines/timeplt/timeplt.h"
#endif

#ifdef ENABLE_TUTANKHM
  #include "machines/tutankhm/tutankhm.h"
#endif

#ifdef ENABLE_SPACE 
  #include "machines/spaceinvaders/spaceinvaders.h"
#endif

#ifdef ENABLE_GALAXIAN 
  #include "machines/galaxian/galaxian.h"
#endif

#ifdef ENABLE_STARFORCE
  #include "machines/starforce/starforce.h"
#endif

#ifdef ENABLE_MOONCRESTA
  #include "machines/mooncresta/mooncresta.h"
#endif

#ifdef ENABLE_SCRAMBLE
  #include "machines/scramble/scramble.h"
#endif

#ifdef ENABLE_SUPERCOBRA
  #include "machines/supercobra/supercobra.h"
#endif

#ifdef ENABLE_DKONG3
  #include "machines/dkong3/dkong3.h"
#endif

#ifdef ENABLE_SUPERCOBRA
  #include "machines/supercobra/supercobra.h"
#endif
// ── Array de punteros — se rellena en initMachines() ─────────
// NO se usan 'new' aquí para evitar inicialización estática
// antes de que el heap de Bluepad32 esté listo.
// Tamaño máximo = número de juegos compilados
#define MAX_MACHINES 32
extern machineBase *machines[MAX_MACHINES];
extern signed char machinesCount;

// Llama a esta función al principio de setup(), antes de cualquier
// otra inicialización, para crear los objetos de juego en el heap.
inline void initMachines() {
  machinesCount = 0;
#ifdef ENABLE_PACMAN
  machines[machinesCount++] = new pacman();
#endif
#ifdef ENABLE_GALAGA
  machines[machinesCount++] = new galaga();
#endif
#ifdef ENABLE_DIGDUG
  machines[machinesCount++] = new digdug();
#endif
#ifdef ENABLE_FROGGER
  machines[machinesCount++] = new frogger();
#endif
#ifdef ENABLE_DKONG
  machines[machinesCount++] = new dkong();
#endif
#ifdef ENABLE_1942
  machines[machinesCount++] = new _1942();
#endif
#ifdef ENABLE_LIZWIZ
  machines[machinesCount++] = new lizwiz();
#endif
#ifdef ENABLE_EYES
  machines[machinesCount++] = new eyes();
#endif
#ifdef ENABLE_MRTNT
  machines[machinesCount++] = new mrtnt();
#endif
#ifdef ENABLE_THEGLOB
  machines[machinesCount++] = new theglob();
#endif
#ifdef ENABLE_CRUSH
  machines[machinesCount++] = new crush();
#endif
#ifdef ENABLE_ANTEATER
  machines[machinesCount++] = new anteater();
#endif
#ifdef ENABLE_BOMBJACK
  machines[machinesCount++] = new bombjack();
#endif
#ifdef ENABLE_MRDO
  machines[machinesCount++] = new mrdo();
#endif
#ifdef ENABLE_BAGMAN
  machines[machinesCount++] = new bagman();
#endif
#ifdef ENABLE_PENGO
  machines[machinesCount++] = new pengo();
#endif
#ifdef ENABLE_GYRUSS
  machines[machinesCount++] = new gyruss();
#endif
#ifdef ENABLE_MSPACMAN
  machines[machinesCount++] = new mspacman();
#endif
#ifdef ENABLE_GALAXIAN
  machines[machinesCount++] = new galaxian();
#endif
#ifdef ENABLE_SPACE
  machines[machinesCount++] = new spaceinvaders();
#endif
#ifdef ENABLE_TIMEPLT
  machines[machinesCount++] = new timeplt();
#endif
#ifdef ENABLE_TUTANKHM
  machines[machinesCount++] = new tutankhm();
#endif
#ifdef ENABLE_DKONGJR
  machines[machinesCount++] = new dkongjr();
#endif
#ifdef ENABLE_LADYBUG
  machines[machinesCount++] = new ladybug();
#endif
#ifdef ENABLE_STARFORCE
  machines[machinesCount++] = new starforce();
#endif
#ifdef ENABLE_MOONCRESTA
  machines[machinesCount++] = new mooncresta();
#endif
#ifdef ENABLE_SCRAMBLE
  machines[machinesCount++] = new scramble();
#endif
#ifdef ENABLE_SUPERCOBRA
  machines[machinesCount++] = new supercobra();
#endif
  static_assert(MAX_MACHINES >= 32, "MAX_MACHINES demasiado pequeño");
}

#endif // _MACHINES_H_