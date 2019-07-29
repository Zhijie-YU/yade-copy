# Create docker images for yade

Important is to have in the current directory two gpg-keys to sign Yade package:
   * yadedev_pub.gpg
   * yadedev_sec.gpg

Those keys should be in a ASCII-format and with the removed password

# Run docker-container

## Create initial repos in the /home/anton/dem/yade/aptly

docker run --rm  --mount 'type=bind,source=/home/anton/dem/yade/aptly,target=/root/.aptly' aptly:yade bash /root/update_repos_next.sh

## First package upload
docker run --rm  --mount 'type=bind,source=/home/anton/dem/yade/aptly,target=/root/.aptly' -e "AWS_ACCESS_KEY_ID=XXX" -e "AWS_SECRET_ACCESS_KEY=XXX" aptly:yade bash /root/update_repos_next.sh

## Following package uploads
docker run --rm  --mount 'type=bind,source=/home/anton/dem/yade/aptly,target=/root/.aptly' -e "AWS_ACCESS_KEY_ID=XXX" -e "AWS_SECRET_ACCESS_KEY=XXX" aptly:yade bash /root/update_repos_next.sh