#!/bin/sh

if [ -f release/disk.vdi ]; then
    VBoxManage unregistervm myos --delete
    rm -f release/disk.vdi
fi
VBoxManage convertfromraw release/kernel.iso release/disk.vdi
VBoxManage createvm --name myos --register
VBoxManage modifyvm myos --acpi on --ioapic on --cpus 2 --memory 128 --boot1 disk --boot2 none --boot3 none --boot4 none
VBoxManage storagectl myos --name "SATA Controller" --add sata
VBoxManage storageattach myos --storagectl "SATA Controller" --port 0 --type hdd --medium release/disk.vdi
VBoxManage startvm myos
