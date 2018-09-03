 
 
sudo apt-get install python-pip python-dev build-essential 
sudo pip install --upgrade pip 
sudo pip install --upgrade virtualenv 
sudo pip install recommonmark

sudo pip install sphinx
sudo pip install sphinx_rtd_theme

cd ../Docs
make html
