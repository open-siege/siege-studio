import darkstar
import re
from functools import partial

game = darkstar.Game.currentInstance()

plugins = game.starsiegePlugins
console = game.console

console.echo("Hello world from Python in simple.py")

def csExport(exportName):
    def decorator(func):
        func.exportName = exportName
        return func

    return decorator

class PythonPlugin(darkstar.PyConsoleCallback):
    callbacks = {}
    exports = []

    def addCommand(self, funcName, func):
        console.echo(f"adding a command called {funcName} {len(self.callbacks)}");
        self.callbacks[funcName] = func

        func.localName = funcName
        if not hasattr(func, "exportName"):
            func.exportName = funcName

        self.exports.append(func)

    def removeCommand(self, funcName):
        if funcName in self.callbacks:
            command = self.callbacks[funcName]
            del self.callbacks[funcName]

            for export in self.exports:
                if command.localName == export.name:
                    self.exports.remove(export)
                    break

    @csExport("python::eval")
    def doEval(self, args: list):
        console.echo(f"Calling Python eval with args[1]");
        return eval(args[1], {
            "darkstar": darkstar,
            "game": game,
            "console": console,
            "plugins": plugins
        }, self.callbacks)

    def attach(self, newConsole):
        for index, export in enumerate(self.exports):
            newConsole.echo(f"Adding a callback called {export.exportName}")
            newConsole.addCommand(index, export.exportName, self, 0)

    def doExecuteCallback(self, cmd, callbackId, args: list):
        export = self.exports[callbackId]

        console.echo(F"Calling Python callback {callbackId} {str(args)}");
        if len(args) > 0:
            # optimisation because we don't need to parse the strings
            # eval will do that for us
            if args[0] == "python::eval":
                return str(export(args))

            newArgs = []
            arg: str
            for arg in args:
                if arg.isnumeric():
                    newArgs.append(int(arg, None, None))
                    continue

                if re.match(r'^-?\d+(?:\.\d+)?$', arg) is not None:
                    newArgs.append(float(arg, None, None))
                    continue

                newArgs.append(arg)

            return str(export(newArgs))

        return "False"


console.echo("Adding new callbacks")

callback = PythonPlugin()

doEval = partial(PythonPlugin.doEval, callback)

doEval.exportName = "python::eval"

callback.addCommand("gameEval", doEval)

for index, export in enumerate(callback.exports):
    console.echo(f"Adding a callback called {export.exportName}")
    console.addCommand(index, export.exportName, callback, 0)

console.echo("Done adding a callback")
