<<<<<<< HEAD
/*
 * Example Contiki test script (JavaScript).
 * A Contiki test script acts on mote output, such as via printf()'s.
 * The script may operate on the following variables:
 *  Mote mote, int id, String msg
 */

TIMEOUT(2000, log.log("last message: " + msg + "\n"));

WAIT_UNTIL(msg.equals('Hello, world'));
log.testOK();
=======
/*
 * Example Contiki test script (JavaScript).
 * A Contiki test script acts on mote output, such as via printf()'s.
 * The script may operate on the following variables:
 *  Mote mote, int id, String msg
 */

TIMEOUT(2000, log.log("last message: " + msg + "\n"));

WAIT_UNTIL(msg.equals('Hello, world'));
log.testOK();
>>>>>>> 1c8a365a388f3826ae65a4404b1caaf07e71bb24