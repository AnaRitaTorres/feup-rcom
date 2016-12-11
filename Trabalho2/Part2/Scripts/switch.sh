configure terminal 
vlan 40
end

configure terminal 
vlan 41
end

configure terminal
interface fastethernet 0/1
switchport mode access
switchport access vlan 40
end

configure terminal
interface fastethernet 0/3
switchport mode access
switchport acess vlan 40
end

