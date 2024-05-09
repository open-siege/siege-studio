import importlib.util

def loadPostInstallPlugins(config):
    plugins = []
    if "plugins" in config and "postinstall" in config["plugins"]:
        for key, path in config["plugins"]["postinstall"].items():
            spec = importlib.util.spec_from_file_location(key, path)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            plugins.append(module)

    return plugins

