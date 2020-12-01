#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <ldap.h>
#include "../header/mypw.h"

using namespace std;

int main () {

   const char* ldapUri = "ldap://ldap.technikum-wien.at:389";
   const int ldapVersion = LDAP_VERSION3;

   string username;
   cout << "Username: ";
   getline(cin, username);
   const char* rawLdapUsername = username.c_str();
   char ldapBindUser[256];
   sprintf(ldapBindUser, "uid=%s,ou=people,dc=technikum-wien,dc=at", rawLdapUsername);
   
 // read password (bash: export ldappw=<yourPW>)
   char ldapBindPassword[256];
   strcpy(ldapBindPassword, getpass());
   printf("pw taken over from commandline\n");

   const char* ldapSearchBaseDomainComponent = "dc=technikum-wien,dc=at";
   const char* ldapSearchFilter = "(uid=if19b16*)";
   ber_int_t ldapSearchScope = LDAP_SCOPE_SUBTREE;
   const char* ldapSearchResultAttributes[] = {"uid", "cn", nullptr};

   int rc;

   LDAP* ldapHandle;
   rc = ldap_initialize(&ldapHandle, ldapUri);
   if (rc != LDAP_SUCCESS)
   {
      cout << "ldap_init failed!\n";
   } 
   cout << "connected to LDAP\n" << ldapUri;
   
   rc = ldap_set_option(
      ldapHandle,
      LDAP_OPT_PROTOCOL_VERSION,
      &ldapVersion);
   if (rc != LDAP_OPT_SUCCESS) 
   {
      ldap_unbind_ext_s(ldapHandle, nullptr,nullptr);
      cout << "\nset option failed!\n";
      return EXIT_FAILURE;
   }

   rc = ldap_start_tls_s(
      ldapHandle,
      nullptr,
      nullptr
   );
   if (rc != LDAP_SUCCESS) 
   {
      ldap_unbind_ext_s(ldapHandle, nullptr,nullptr);
      cout << "\nstart tls failed!\n";
      return EXIT_FAILURE;
   }

   BerValue bindCredentials;
   bindCredentials.bv_val = ldapBindPassword;
   bindCredentials.bv_len = strlen(ldapBindPassword);
   BerValue *servercredp; // server's credentials
   rc = ldap_sasl_bind_s(
       ldapHandle,
       ldapBindUser,
       LDAP_SASL_SIMPLE,
       &bindCredentials,
       NULL,
       NULL,
       &servercredp);
   if (rc !=  LDAP_SUCCESS)
   {
      ldap_unbind_ext_s(ldapHandle, nullptr, nullptr);
      cout << "\nsasl_bind failed!\n";
      return EXIT_FAILURE;
   }

 ////////////////////////////////////////////////////////////////////////////
   // perform ldap search
   // https://linux.die.net/man/3/ldap_search_ext_s
   // _s : synchronous
   // int ldap_search_ext_s(
   //     LDAP *ld,
   //     char *base,
   //     int scope,
   //     char *filter,
   //     char *attrs[],
   //     int attrsonly,
   //     LDAPControl **serverctrls,
   //     LDAPControl **clientctrls,
   //     struct timeval *timeout,
   //     int sizelimit,
   //     LDAPMessage **res );
   LDAPMessage *searchResult;
   rc = ldap_search_ext_s(
       ldapHandle,
       ldapSearchBaseDomainComponent,
       ldapSearchScope,
       ldapSearchFilter,
       (char **)ldapSearchResultAttributes,
       0,
       NULL,
       NULL,
       NULL,
       500,
       &searchResult);
   if (rc != LDAP_SUCCESS)
   {
      ldap_unbind_ext_s(ldapHandle, NULL, NULL);
      cout << "\nldap search failed!\n";
      return EXIT_FAILURE;
   }
   // https://linux.die.net/man/3/ldap_count_entries
   printf("Total results: %d\n", ldap_count_entries(ldapHandle, searchResult));

   ////////////////////////////////////////////////////////////////////////////
   // get result of search
   // https://linux.die.net/man/3/ldap_first_entry
   // https://linux.die.net/man/3/ldap_next_entry
   LDAPMessage *searchResultEntry;
   for (searchResultEntry = ldap_first_entry(ldapHandle, searchResult);
        searchResultEntry != NULL;
        searchResultEntry = ldap_next_entry(ldapHandle, searchResultEntry))
   {
      /////////////////////////////////////////////////////////////////////////
      // Base Information of the search result entry
      // https://linux.die.net/man/3/ldap_get_dn
      printf("DN: %s\n", ldap_get_dn(ldapHandle, searchResultEntry));

      /////////////////////////////////////////////////////////////////////////
      // Attributes
      // https://linux.die.net/man/3/ldap_first_attribute
      // https://linux.die.net/man/3/ldap_next_attribute
      //
      // berptr: berptr, a pointer to a BerElement it has allocated to keep
      //         track of its current position. This pointer should be passed
      //         to subsequent calls to ldap_next_attribute() and is used to
      //         effectively step through the entry's attributes.
      BerElement *ber;
      char *searchResultEntryAttribute;
      for (searchResultEntryAttribute = ldap_first_attribute(ldapHandle, searchResultEntry, &ber);
           searchResultEntryAttribute != NULL;
           searchResultEntryAttribute = ldap_next_attribute(ldapHandle, searchResultEntry, ber))
      {
         BerValue **vals;
         if ((vals = ldap_get_values_len(ldapHandle, searchResultEntry, searchResultEntryAttribute)) != NULL)
         {
            for (int i = 0; i < ldap_count_values_len(vals); i++)
            {
               printf("\t%s: %s\n", searchResultEntryAttribute, vals[i]->bv_val);
            }
            ldap_value_free_len(vals);
         }

         // free memory
         ldap_memfree(searchResultEntryAttribute);
      }
      // free memory
      if (ber != NULL)
      {
         ber_free(ber, 0);
      }

      printf("\n");
   }

   // free memory
   ldap_msgfree(searchResult);

   ////////////////////////////////////////////////////////////////////////////
   // https://linux.die.net/man/3/ldap_unbind_ext_s
   // int ldap_unbind_ext_s(
   //       LDAP *ld,
   //       LDAPControl *sctrls[],
   //       LDAPControl *cctrls[]);
   ldap_unbind_ext_s(ldapHandle, NULL, NULL);

   return EXIT_SUCCESS;
}
