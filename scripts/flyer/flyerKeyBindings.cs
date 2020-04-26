// these are all the flyer controls which make everything work
bindAction(keyboard0, make, "q", TO, IDACTION_YAW, 1.000000);
bindAction(keyboard0, break, "q", TO, IDACTION_YAW, 0.000000);

bindAction(keyboard0, make, "e", TO, IDACTION_YAW, -1.000000);
bindAction(keyboard0, break, "e", TO, IDACTION_YAW, 0.000000);

bindAction(keyboard0, make, shift, "q", TO, IDACTION_ROLL, 0.500000);
bindAction(keyboard0, break, shift, "q", TO, IDACTION_ROLL, 0.000000);

bindAction(keyboard0, make, shift, "e", TO, IDACTION_ROLL, -0.500000);
bindAction(keyboard0, break, shift, "e", TO, IDACTION_ROLL, 0.000000);


bindAction(keyboard0, make, "r", TO, IDACTION_PITCH, 0.500000);
bindAction(keyboard0, break, "r", TO, IDACTION_PITCH, 0.000000);
bindAction(keyboard0, make, "f", TO, IDACTION_PITCH, -0.500000);
bindAction(keyboard0, break, "f", TO, IDACTION_PITCH, 0.000000);


bindCommand(keyboard0, make, "space", TO, "remoteEval(2048, FlyerJump, \"start\", 1);");
bindCommand(keyboard0, break, "space", TO, "remoteEval(2048, FlyerJump, \"stop\", 0);");

bindCommand(keyboard0, make, "c", TO, "remoteEval(2048, FlyerJump, \"start\", -1);");
bindCommand(keyboard0, break, "c", TO, "remoteEval(2048, FlyerJump, \"stop\", 0);");

bindCommand(keyboard0, make, "a", TO, "remoteEval(2048, FlyerStrafe, \"start\", -1);");
bindCommand(keyboard0, break, "a", TO, "remoteEval(2048, FlyerStrafe, \"stop\", 0);");

bindCommand(keyboard0, make, "d", TO, "remoteEval(2048, FlyerStrafe, \"start\", 1);");
bindCommand(keyboard0, break, "d", TO, "remoteEval(2048, FlyerStrafe, \"stop\", 0);");

bindCommand(keyboard0, make, "w", TO, "remoteEval(2048, FlyerAccel, \"start\", 1);");
bindCommand(keyboard0, break, "w", TO, "remoteEval(2048, FlyerAccel, \"stop\", 0);");

bindCommand(keyboard0, make, "s", TO, "remoteEval(2048, FlyerAccel, \"start\", -1);");
bindCommand(keyboard0, break, "s", TO, "remoteEval(2048, FlyerAccel, \"stop\", 0);");