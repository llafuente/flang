# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "relativkreativ/centos-7-minimal"

  config.vm.network :forwarded_port, guest: 22, host: 2701, id: "ssh", auto_correct: true

  config.vm.provider :virtualbox do |vb|
    # Don't boot with headless mode
    #vb.gui = true

    vb.customize ["modifyvm", :id, "--memory", "1024"]
  end

  config.vm.provision "shell" do |s|
    s.path = "vagrant-boot.sh"
  end
end
