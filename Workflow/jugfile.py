from jug import TaskGenerator

import subprocess
import sys

@TaskGenerator
def ProcessScan(scanID):
    try:
        subprocess.call(['/usr/bin/python', '/home/cory/code/src/Workflow/Examples/PediatricAirways/PediatricAirwaysWorkflow.py', '%d' % scanID, '--verbose'])
    except Exception as ex:
        sys.stdout.write('Error processing %d\n' % scanID)
        sys.stdout.write(str(ex))

crl = [ProcessScan(id) for id in range(1000, 1193)]
sgs = [ProcessScan(id) for id in range(2000, 2028)]
prs = [ProcessScan(id) for id in range(3000, 3040)]
