# typewriter-interface

Check what is running
ps -aux |grep 

Kill what is running
sudo kill -9 

Run python in background
nohup python nameoffile.py > output.log &

Run ngrok in background
./ngrok http 5000 > /dev/nul &

Find ngrok url
curl localhost:4040/status
or
curl http://127.0.0.1:4040/api/tunnels 




