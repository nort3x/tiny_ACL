rm -r test
mkdir test
make build
mv auth test
cd test

NEWLINE=$'\n'
echo "fixing file permissions..."
chmod +x *
echo "cleaning ..."
./clear.sh

echo "##########Adding Users with Password##########"

for i in {1..100}
do
echo "adding user$i"
result=$(./auth AddUser user$i 23456789$i)
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done


echo "##########Adding Users without Password##########"

for i in {1..100}
do
echo "adding user$i"
result=$(./auth AddUser passwordlessuser$i )
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done




echo "##########Authenticating Users with Password##########"

for i in {1..100}
do
echo "authenticate user$i"
result=$(./auth Authenticate user$i 23456789$i)
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done




echo "##########Authenticating Users without Password##########"

for i in {1..100}
do
echo "authenticate user$i"
result=$(./auth Authenticate passwordlessuser$i)
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########Authenticating Non-Existing Users##########"

for i in {1..100}
do
echo "authenticate junkuser$i"
result=$(./auth Authenticate junkuser$i junk$i)
if [[ "$result" != "Error: no such user" ]]
then
echo "test failed "
exit 0
fi
done




echo "##########Authenticating Users with Bad Password##########"

for i in {1..100}
do
echo "authenticate user$i"
result=$(./auth Authenticate user$i junk$i)
if [[ "$result" != "Error: bad password" ]]
then
echo "test failed "
exit 0
fi
done


echo "##########SetDomain for Users##########"

for i in {1..100}
do
echo "SetDomain user$i userdomain$i"
result=$(./auth SetDomain user$i userdomain$i)
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done


echo "##########SetDomain for Non-Existing Users##########"

for i in {1..100}
do
echo "SetDomain junkuser$i userdomain$i"
result=$(./auth SetDomain junkuser$i userdomain$i)
if [[ "$result" != "Error: no such user" ]]
then
echo "test failed "
exit 0
fi
done


echo "##########SetDomain for Empty Domain##########"

for i in {1..100}
do
echo "SetDomain user$i "
result=$(./auth SetDomain user$i)
if [[ "$result" != "Error: missing domain" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########SetDomain for Repeating Domain##########"

for i in {1..100}
do
echo "SetDomain user$i userdomain$i"
result=$(./auth SetDomain user$i userdomain$i)
if [[ "$result" != "Success"  ]]
then
echo "test failed "
exit 0
fi
done


echo "##########DomainInfo for Domain and Repatation Check##########"

for i in {1..100}
do
echo "SetDomain user$i "
result=$(./auth DomainInfo userdomain$i)
if [[ "$result" != "user$i" ]]
then
echo "test failed "
exit 0
fi
done




echo "##########Adding Co_users##########"

for i in {1..100}
do
echo "adding couser$i"
result=$(./auth AddUser couser$i 0123456789 )
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########SetDomain for co-users##########"

for i in {1..100}
do
echo "SetDomain couser$i userdomain$i"
result=$(./auth SetDomain couser$i userdomain$i)
if [[ "$result" != "Success"  ]]
then
echo "test failed "
exit 0
fi
done



echo "##########DomainInfo Check for Co-Users ##########"

for i in {1..100}
do
echo "DomainInfo userdomain$i"
result=$(./auth DomainInfo userdomain$i)
if [[ "$result" != "user${i}${NEWLINE}couser$i" ]]
then
echo "test failed "
exit 0
fi
done




echo "##########DomainInfo Check for Non-Existing Domain##########"

for i in {1..100}
do
echo "DomainInfo junkdomain$i"
result=$(./auth DomainInfo junkdomain$i)
if [[ "$result" != "" ]]
then
echo "test failed "
exit 0
fi
done





echo "##########SetType Assigning to object##########"

for i in {1..100}
do
echo "SetType object$i type$i"
result=$(./auth SetType object$i type$i)
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done




echo "##########SetType Assigning to Empty Type##########"

for i in {1..100}
do
echo "SetType object$i"
result=$(./auth SetType object$i)
if [[ "$result" != "Error: missing type" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########TypeInfo for Types##########"

for i in {1..100}
do
echo "TypeInfo type$i"
result=$(./auth TypeInfo type$i)
if [[ "$result" != "object$i" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########TypeInfo for Non-Existing Types##########"

for i in {1..100}
do
echo "TypeInfo junktype$i"
result=$(./auth TypeInfo junktype$i)
if [[ "$result" != "" ]]
then
echo "test failed "
exit 0
fi
done


echo "##########TypeInfo for Empty Types##########"

for i in {1..100}
do
echo "TypeInfo "
result=$(./auth TypeInfo )
if [[ "$result" != "Error: missing type" ]]
then
echo "test failed "
exit 0
fi
done


echo "##########SetType Assigning to Co-Object##########"

for i in {1..100}
do
echo "SetType object$i type$i"
result=$(./auth SetType coobject$i type$i)
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########TypeInfo for Types Co-Object Existing Check##########"

for i in {1..100}
do
echo "TypeInfo type$i"
result=$(./auth TypeInfo type$i)
if [[ "$result" != "object${i}${NEWLINE}coobject$i" ]]
then
echo "test failed "
exit 0
fi
done




echo "##########AddAccess for Domain and Type##########"

for i in {1..100}
do
echo "AddAccess  operation${i} domain${i} type${i}"
result=$(./auth AddAccess operation${i} domain${i} type${i})
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########AddAccess for Non-Existing Domain and Non-Existing Type##########"

for i in {1..100}
do
echo "AddAccess  operation${i} primedomain${i} primetype${i}"
result=$(./auth AddAccess operation${i} primedomain${i} primetype${i})
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########AddAccess for Empty Operation##########"

for i in {1..100}
do
echo "AddAccess"
result=$(./auth AddAccess)
if [[ "$result" != "Error: missing operation" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########AddAccess for Empty Operation##########"

for i in {1..100}
do
echo "AddAccess operation${i}"
result=$(./auth AddAccess operation${i})
if [[ "$result" != "Error: missing domain" ]]
then
echo "test failed "
exit 0
fi
done




echo "##########CanAccess for Object and Domain##########"

for i in {1..100}
do
echo "CanAccess  operation${i} user$i object$i"
result=$(./auth CanAccess operation${i} user$i object$i)
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi
done


echo "##########CanAccess (bad user) for Object and Domain##########"

for i in {1..100}
do
echo "CanAccess  operation${i} user1 object2"
result=$(./auth CanAccess  operation${i} user1 object2)
if [[ "$result" != "Error: access denied" ]]
then
echo "test failed "
exit 0
fi
done



echo "##########CanAccess Cross Domin Access##########"

./auth SetType object1 lux
./auth AddAccess operation1 userdomain2 lux 
 result=$(./auth CanAccess operation1  user2 object1)
if [[ "$result" != "Success" ]]
then
echo "test failed "
exit 0
fi

