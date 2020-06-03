import darkstar

game = darkstar.currentInstance()

console = game.getConsole()

console.eval('echo("Hello world from Python in simple.py");')

with open("hello-python.log", "w") as newFile:
    newFile.write("Hello world from Python through Starsiege")