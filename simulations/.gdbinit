python
import subprocess
omnetPath = subprocess.check_output('echo -n $(dirname $(which omnetpp))/../', shell=True)
__file__ = omnetPath + 'misc/gdb/gdbinit.py'
execfile(__file__)
end
