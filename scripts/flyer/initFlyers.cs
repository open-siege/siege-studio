exec("flyer\\flyerCommands.cs");

focusServer();

// Enable all of the flyers present in our settings.
for (%i = 0; $flyer::availableFlyerNames[%i] != ""; %i++) 
{
		%name = $flyer::availableFlyerNames[%i];
		%flyerId = $flyer::availableFlyerIds[%name];
		if (%flyerId != 0)
		{
			echo(strcat(%name, " now enabled on your server."));
			allowVehicle(%flyerId, true );
		}
}

echo("All done. Clients need flyer_Keyboard.cs or flyer_Keyboard_Mouse.cs to enjoy properly.");

focusClient();