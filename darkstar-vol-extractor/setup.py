from setuptools import setup, find_packages

with open("README.md", "r") as fh:
    long_description = fh.read()

print(find_packages())
setup(
    name='darkstar-vol-extractor',
    version='0.0.4',
    packages=find_packages(),
    url='https://github.com/matthew-rindel/darkstar-vol-extractor',
    license='MIT',
    author='Matthew',
    author_email='matthew@sobantu.co.za',
    description='',
    py_modules=["volinfo", "unvol", "extract_file"],
    long_description=long_description,
    long_description_content_type="text/markdown"
)
