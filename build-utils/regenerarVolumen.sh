rm -rf fat32.disk 
mkfs.msdos -F 32 -S 512 -s 8 -C fat32.disk 524288
sudo mount -o loop fat32.disk fuseMount/
cd fuseMount/
#sudo nano asd.txt
cd ..
echo "hello world!" > asd.txt
sudo umount -f fuseMount
