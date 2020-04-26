// flyerCommands.cs - Copyright (c) 2020 Matthew Rindel

// If you are looking at this, then check out the
// GitHub page it came from: https://github.com/matthew-rindel/starsiege-flyer-movement

// All of this code helps implement missing movement functionality for flyers in Starsiege.
// Gameplay for flyers would be possible and is very close to what can be found in games such as:
// * Tribes Vengeance (which is a good in-universe reference, as it is a spin-off sequel)
// * Unreal Tournament 2004 (which has the same base engine as Tribes Vengeance)
// * Incoming Forces (a vehicle combat simulator which has multiple types of vehicles)
// There is still some work to be done to make it a better experience overall, but so far, it serves as an minimal version to build off of.

// We need to implement functions found in Tribes, but not Starsiege
exec("math\\shim.cs");

// Super useful and well organised. Minimal shims added above to make the dependencies work.
// More shims would be needed for other functions.
exec("TRIBES Math Library.cs");

// Get our trusty settings
exec("flyer\\flyerSettings.cs");


function flyer::isFlyer(%vehicleId)
{
   for (%i = 0; $flyer::availableFlyerNames[%i] != ""; %i++) 
   {
		%name = getVehicleName(%vehicleId);
		if (%name == $flyer::availableFlyerNames[%i])
		{
			return true;
		}
   }
   return false;
}


function flyer::translateByRotation(%vehicle, %rotationInDegrees, %translation)
{
	%hercX = getPosition(%vehicle, X);
	%hercY = getPosition(%vehicle, Y);
	%hercZ = getPosition(%vehicle, Z);
	%newRotation = Math::deg2rad(%rotationInDegrees);

	%newX = %hercX + Math::cos(%newRotation) * %translation - Math::sin(%newRotation) * %translation;
	%newY = %hercY + Math::sin(%newRotation) * %translation + Math::cos(%newRotation) * %translation;
		
	setPosition(%vehicle, %newX, %newY , %hercZ);	
}

function flyer::updateZPosition(%vehicle, %value)
{
	%hercX = getPosition(%vehicle, X);
	%hercY = getPosition(%vehicle, Y);
	%hercZ = getPosition(%vehicle, Z) + %value;
	%rot = getPosition(%vehicle, rot);
	setPosition(%vehicle, %hercX, %hercY, %hercZ, 0, %rot);
}

function flyer::doJump(%player, %firstJump, %value)
{
	if (%value > 1)
	{
		%value = 1;
	}
	
	if (%value < -1)
	{
		%value = -1;
	}
	
	if (%value >= 0)
	{
		%value = $flyer::baseAcceleration["up"] + %value;	
		echo(strcat("Going up by: ", %value));
	}
	else
	{
		%value = $flyer::baseAcceleration["down"] + %value;	
		echo(strcat("Going down by: ", %value));
	}
	
	%vehicle = playerManager::playerNumToVehicleId(%player);
	
	if (%vehicle.accelerating != true)
	{
		%rot = getPosition(%vehicle, rot);	
		%degrees = Math::rad2deg(%rot);
			
		%vehicle.currentRotation = %rot;	
		%vehicle.currentRotationInDegrees = %degrees;
	}
	
	if(%firstJump && !%vehicle.jumping)
	{
		%vehicle.jumping = True;
	}
	
	if (%vehicle.jumping)
	{		
		flyer::updateZPosition(%vehicle, %value);
		schedule("flyer::doJump("@%player@", False, "@%value@");", $flyer::pollingRate);
	}
	
	return;
}

function flyer::doStrafe(%player, %firstJump, %value)
{
	if (%value > 1)
	{
		%value = 1;
	}
	
	if (%value < -1)
	{
		%value = -1;
	}
	
	if (%value >= 0)
	{
		%value = $flyer::baseAcceleration["right"] + %value;	
		echo(strcat("Going right by: ", %value));
	}
	else
	{
		%value = $flyer::baseAcceleration["left"] + %value;	
		echo(strcat("Going left by: ", %value));
	}
	
	%vehicle = playerManager::playerNumToVehicleId(%player);
	
	if (%vehicle.accelerating != true)
	{
		%rot = getPosition(%vehicle, rot);	
		%degrees = Math::rad2deg(%rot);
			
		%vehicle.currentRotation = %rot;	
		%vehicle.currentRotationInDegrees = %degrees;
	}
	
	if(%firstJump && !%vehicle.strafing)
	{
		%vehicle.strafing = True;
	}
	
	if (%vehicle.strafing)
	{	
		%newAngle = flyer::_adjustAngle(%vehicle.currentRotationInDegrees);
		
		flyer::translateByRotation(%vehicle, %newAngle, %value);
		schedule("flyer::doStrafe("@%player@", False, "@%value@");", $flyer::pollingRate);
	}
	
	return;
}

function flyer::_adjustAngle(%angle, %adjustment)
{
	%result = %angle + %adjustment;
	
	if (%result > 360)
	{
		%result = 360;
	}
	
	if (%result > 180)
	{
		%result = -1 *(360 - %result);
	}
	
	if (%result < -180)
	{
		%result = 360 + %result;
	}
	
	return %result;
}


function flyer::doAccel(%player, %firstJump, %value)
{
	if (%value > 1)
	{
		%value = 1;
	}
	
	if (%value < -1)
	{
		%value = -1;
	}
	
	if (%value >= 0)
	{
		%value = $flyer::baseAcceleration["forward"] + %value;	
		echo(strcat("Going forward by: ", %value));
	}
	else
	{
		%value = $flyer::baseAcceleration["backward"] + %value;	
		echo(strcat("Going backward by: ", %value));
	}
	
	%vehicle = playerManager::playerNumToVehicleId(%player);
	
	if(%firstJump && !%vehicle.accelerating)
	{
		%vehicle.accelerating = True;
	}
	
	if (%vehicle.accelerating)
	{		
		%rot = getPosition(%vehicle, rot);
		
		%degrees = Math::rad2deg(%rot);
			
		if (%vehicle.currentRotation == 0)
		{
			%vehicle.currentRotation = %rot;	
			%vehicle.currentRotationInDegrees = %degrees;
		}
			
		%absDegrees = Math::abs(%degrees);
		%absCurrentDegrees = Math::abs(%vehicle.currentRotationInDegrees);
			
		if (%absDegrees >= %absCurrentDegrees + 10 || %absDegrees >= %absCurrentDegrees - 10)
		{
			%vehicle.currentRotation = %rot;	
			%vehicle.currentRotationInDegrees = %degrees;
			echo(%degrees);
		}
		
		%newAngle = flyer::_adjustAngle(%vehicle.currentRotationInDegrees, 45);
		
		flyer::translateByRotation(%vehicle, %newAngle, %value);
		
		
		schedule("flyer::doAccel("@%player@", False, "@%value@");", $flyer::pollingRate);
	}
	
	return;
}

function remoteFlyerAccel(%player, %action, %value) 
{
	%vehicle = playerManager::playerNumToVehicleId(%player);
	if (!flyer::isFlyer(%vehicle))
	{
		return;
	}
	
	if (%action == "start")
	{
		flyer::doAccel(%player, True, %value, say);	
	}
	else if (%action == "stop")
	{
		%vehicle.accelerating = False;		
	}
}

function remoteFlyerJump(%player, %action, %value) 
{
	%vehicle = playerManager::playerNumToVehicleId(%player);
	
	if (!flyer::isFlyer(%vehicle))
	{
		return;
	}

	
	if (%action == "start")
	{
		flyer::doJump(%player, True, %value);	
	}
	else if (%action == "stop")
	{
		%vehicle.jumping = False;		
	}
}

function remoteFlyerStrafe(%player, %action, %value) 
{
	%vehicle = playerManager::playerNumToVehicleId(%player);
	
	if (!flyer::isFlyer(%vehicle))
	{
		return;
	}
	
	if (%action == "start")
	{
		flyer::doStrafe(%player, True, %value);	
	}
	else if (%action == "stop")
	{
		%vehicle = playerManager::playerNumToVehicleId(%player);
		%vehicle.strafing = False;		
	}
}