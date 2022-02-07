exec("Keyboard_Mouse.cs");

editActionMap(Herc);
exec("flyer\\flyerKeyBindings.cs");
bindAction( mouse0, xaxis,   TO, IDACTION_YAW, scale, 0.25, flip ); 
bindAction( mouse0, yaxis,   TO, IDACTION_PITCH, scale, 0.25, flip ); 