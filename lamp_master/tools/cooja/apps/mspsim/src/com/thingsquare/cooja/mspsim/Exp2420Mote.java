<<<<<<< HEAD
package com.thingsquare.cooja.mspsim;

import java.io.File;

import se.sics.cooja.Simulation;
import se.sics.cooja.mspmote.Exp5438Mote;
import se.sics.cooja.mspmote.MspMoteType;

/**
 * @author Fredrik Osterlind
 */
public class Exp2420Mote extends Exp5438Mote {
  private String desc = "";
  
  public Exp2420Mote(MspMoteType moteType, Simulation sim) {
    super(moteType, sim);
  }

  protected boolean initEmulator(File fileELF) {
    return super.initEmulator(fileELF);
  }

  public String toString() {
    return desc + " " + getID();
  }
}
=======
package com.thingsquare.cooja.mspsim;

import java.io.File;

import se.sics.cooja.Simulation;
import se.sics.cooja.mspmote.Exp5438Mote;
import se.sics.cooja.mspmote.MspMoteType;

/**
 * @author Fredrik Osterlind
 */
public class Exp2420Mote extends Exp5438Mote {
  private String desc = "";
  
  public Exp2420Mote(MspMoteType moteType, Simulation sim) {
    super(moteType, sim);
  }

  protected boolean initEmulator(File fileELF) {
    return super.initEmulator(fileELF);
  }

  public String toString() {
    return desc + " " + getID();
  }
}
>>>>>>> 1c8a365a388f3826ae65a4404b1caaf07e71bb24