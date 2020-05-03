// shim.cs - Copyright (c) 2020 Matthew Rindel

// If you are looking at this, then check out the
// GitHub page it came from: https://github.com/matthew-rindel/starsiege-math-polyfills

// These are all functions that are present in Starsiege: Tribes but not Starsiege itself.
// In order to make it easier for some more standard functions to work between the games,
// I have added implementations of some of them based on what is available.

// Luckily floor was present, so implmenting ceil can just make use of that.
function ceil(%number)
{
	return floor(%number + 1);
}

// Adapted from a C++ version from here: https://stackoverflow.com/questions/3581528/how-is-the-square-root-function-implemented/39716982
// Often, the values lack precision when dividing, so ceil is used 
// to round up values which may be lacking in precision.
function Math::sqrt(%number){
  %lo = 0;
  %hi = %number;
  %mid = 0;
  for(%i = 0 ; %i < 1000 ; %i++){
      %mid = (%lo + %hi)/2;
	  
      if((%mid * %mid) == %number) 
	  {
		  return %mid;
	  }
	  
	  if((ceil(%mid) * ceil(%mid)) == %number) 
	  {
		  return ceil(%mid);
	  }
	  
      if((%mid * %mid) > %number)
	  {
		  %hi = %mid;
	  }
      else 
	  {
		  %lo = %mid;
	  }
  }
  return %mid;
}

// Raising a number to the power of another is just multiplying it multiple times, which the exception of
// raising things to the power of 0, which would just give you 1.
function pow(%value, %power)
{
	if (%power == 0)
	{
		return 1;
	}
	
	%result = %value;
	
	for (%i = 2; %i <= %power; %i++)
	{
		%result *= %value;
	}
	
	return %result;
}