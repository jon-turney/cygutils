/*
 * Execute a program with a restricted access token.
 *
 * Copyright (C) 2009,2011  Christian Franke
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See the COPYING file for full license information.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif
#define WINVER 0x0500 // >= W2K
#include "common.h"

#include <grp.h>
#include <regex.h>
#include <sddl.h>
#include <sys/cygwin.h>
#include <getopt.h>

#define AUTHORS "Christian Franke"

static const char * progname;
static const char versionID[] = PACKAGE_VERSION;
static void help (FILE * f, const char *name);
static void version (FILE * f, const char *name);
static void license (FILE * f, const char *name);

static int
usageCore (FILE * f, const char * name)
{
  fprintf (f,
    "Usage: %s [OPTIONS] COMMAND [ARG ...]\n"
    "\n"
    "Group options\n"
    "  -l        Disable local administrator group [default]\n"
    "            (same as '-g S-1-5-32-544').\n"
    "  -d        Disable domain administrator group [default]\n"
    "            (same as '-g S-1-5-21-.\\*-512').\n"
    "  -g GROUP  Disable group(s) GROUP.\n"
    "  -G GROUP  Disable all groups except group(s) GROUP.\n"
    "  -r GROUP  Add group(s) GROUP to restricted SIDs.\n"
    "\n"
    "Privilege options\n"
    "  -m        Delete most privileges [default]\n"
    "            (same as '-P SeChangeNotifyPrivilege').\n"
    "  -p PRIV   Delete privilege PRIV.\n"
    "  -P PRIV   Delete all privileges except privilege PRIV.\n"
    "\n"
    "General options\n"
    "  --help,-h Print this help.\n"
    "  --usage   Display brief usage information.\n"
    "  --version Display version information.\n"
    "  --license Display licensing information.\n"
    "  -v        Verbose output, lists groups and privileges changed.\n"
    "            Repeat to list all groups and privileges.\n"
    "\n",
    name);
}

static int
usage ()
{
  usageCore (stderr, progname);
  return 1;
}

static int
usageerror (const char * msg)
{
  fprintf(stderr, "%s: %s\n", progname, msg);
  return 1;
}

static int
cygerror (const char * msg)
{
  perror (msg);
  return 1;
}

static int
winerror (const char * msg)
{
  fprintf (stderr, "%s: error %u\n", msg, (unsigned)GetLastError());
  return 1;
}

static bool
is_strsid (const char * s)
{
  return (!strncmp (s, "S-", 2)
          && strspn (s + 2, "-0123456789") == strlen (s) - 2);
}

static const char *
group_to_strsid (const char * name)
{
  if (is_strsid(name))
    // String SID.
    return name;
  else if (!strncmp (name, "S-", 2))
    // SID regex.
    {
      regex_t rex;
      if (regcomp (&rex, name, REG_EXTENDED))
	{
	  fprintf (stderr, "%s: invalid regex\n", name);
	  return NULL;
	}
      regfree (&rex);
      return name;
    }
  else
    {
      // Cygwin group id or name.
      const struct group * g;
      if (strspn (name, "0123456789") == strlen (name))
	g = getgrgid (atoi (name));
      else
	g = getgrnam (name);
      if (!(g && !strncmp (g->gr_passwd, "S-", 2)))
	{
	  fprintf (stderr, "%s: unknown group\n", name);
	  return NULL;
	}
      return strdup (g->gr_passwd); // TODO: Never freed.
    }
}

static const struct group *
strsid_to_group(const char * strsid)
{
  setgrent ();
  while (const struct group * g = getgrent ())
    if (!strcmp (strsid, g->gr_passwd))
      return g;
  return NULL;
}

static bool
match_strsid (const char * strsid, const char * pattern)
{
  if (is_strsid (pattern))
    return !strcmp (strsid, pattern);
  regex_t rex;
  if (regcomp (&rex, pattern, REG_EXTENDED))
    return false;
  bool rc = false;
  regmatch_t m;
  if (!regexec (&rex, strsid, 1, &m, 0)
       && m.rm_so == 0 && m.rm_eo == (int) strlen (strsid))
    rc = true;
  regfree (&rex);
  return rc;
}

static bool
match_priv (const char * priv, const char * pattern)
{
  if (!strcasecmp (priv, pattern))
    return true;
  char buf[strlen (pattern) + 16];
  sprintf (buf, "Se%sPrivilege", pattern);
  return !strcasecmp (priv, buf);
}

/* use long options for standard options, for
 * consistency with other cygutils programs
 */
#define OPT_USAGE_FLAG     155
#define OPT_VERSION_FLAG   OPT_USAGE_FLAG+1
#define OPT_LICENSE_FLAG   OPT_VERSION_FLAG+1

static struct option long_options[] =
{
  {"help",     no_argument, 0, 'h'},
  {"usage",    no_argument, 0, OPT_USAGE_FLAG},
  {"version",  no_argument, 0, OPT_VERSION_FLAG},
  {"license",  no_argument, 0, OPT_LICENSE_FLAG},
  {0, 0, 0, 0}
};

int
main (int argc, char **argv)
{
  progname = argv[0];
  if (argc < 2)
    return usage ();

  // Parse options.
  const int max_groups = 100, max_privs = 100;
  const char * group_args[max_groups];
  const char * restr_args[max_groups];
  bool restr_used[max_groups] = {false, };
  const char * priv_args[max_privs];
  int num_group_args = 0;
  int num_restr_args = 0;
  int num_priv_args = 0;

  bool d_flag = false, l_flag = false;
  bool m_flag = false;
  bool g_flag = false, G_flag = false;
  bool p_flag = false, P_flag = false;
  bool r_flag = false;
  int verbose = 0;

  int opt;
  int option_index = 0;
  while ((opt = getopt_long(argc, argv, "+dlhmvg:G:p:P:r:",
	                    long_options, &option_index)) != EOF)
    {
      if (num_group_args >= max_groups
	  || num_priv_args >= max_privs
	  || num_restr_args >= max_groups)
	usageerror ("too many options");
      switch (opt)
	{
	case 'd':
	  d_flag = true;
	  break;
	case 'l':
	  l_flag = true;
	  break;
	case 'm':
	  m_flag = true;
	  break;
	case 'g':
	  g_flag = true;
	  if (!(group_args[num_group_args++] = group_to_strsid(optarg)))
	    return 1;
	  break;
	case 'G':
	  G_flag = true;
	  if (!(group_args[num_group_args++] = group_to_strsid(optarg)))
	    return 1;
	  break;
	case 'r':
	  r_flag = true;
	  if (!(restr_args[num_restr_args++] = group_to_strsid(optarg)))
	    return 1;
	  break;
	case 'p':
	  p_flag = true;
	  priv_args[num_priv_args++] = optarg;
	  break;
	case 'P':
	  P_flag = true;
	  priv_args[num_priv_args++] = optarg;
	  break;
	case 'v':
	  verbose++;
	  break;
	case 'h':
	  help (stdout, progname);
	  return 0;
	case OPT_USAGE_FLAG:
	  usageCore (stdout, progname);
	  return 0;
	case OPT_VERSION_FLAG:
	  version (stdout, progname);
	  return 0;
	case OPT_LICENSE_FLAG:
	  license (stdout, progname);
	  return 0;
	case '?':
	  fprintf (stderr, "Try -h for help\n");
	  return 1;
	}
    }

  if (optind >= argc)
    return usageerror ("missing COMMAND");

  // Set defaults
  if (!(d_flag || l_flag || m_flag || g_flag
        || G_flag || p_flag || P_flag || r_flag))
    d_flag = l_flag = m_flag = true;

  if (!G_flag)
    {
      g_flag = true;
      if (l_flag)
        group_args[num_group_args++] = "S-1-5-32-544";
      if (d_flag)
	group_args[num_group_args++] = "S-1-5-21-.*-512";
    }
  else if (l_flag || d_flag || g_flag)
    return usageerror ("'-G' cannot be used with '-l', '-d', '-g'");

  if (!p_flag)
    {
      if (m_flag)
	priv_args[num_priv_args++] = SE_CHANGE_NOTIFY_NAME;
      else if (!P_flag)
	p_flag = true;
    }
  else if (m_flag || P_flag)
    return usageerror ("'-p' cannot be used with '-m', '-P'");

  // Get token
  HANDLE proc_token;
  if(!OpenProcessToken (GetCurrentProcess (), TOKEN_ALL_ACCESS, &proc_token))
    return winerror("OpenProcessToken");

  // Get groups.
  DWORD size = 0;
  if (!GetTokenInformation (proc_token, TokenGroups, NULL, 0, &size)
      && GetLastError () != ERROR_INSUFFICIENT_BUFFER)
    return winerror ("GetTokenInformation");

  char groups_buf[size];
  TOKEN_GROUPS * groups = (TOKEN_GROUPS *)groups_buf;
  if (!GetTokenInformation (proc_token, TokenGroups, groups, sizeof(groups_buf), &size))
    return winerror ("GetTokenInformation");

  // Collect SIDs of groups to disable / restrict.
  SID_AND_ATTRIBUTES sids_to_disable[max_groups];
  SID_AND_ATTRIBUTES sids_to_restrict[max_groups];
  int num_sids_to_disable = 0, num_sids_to_restrict = 0;

  for (DWORD i = 0; i < groups->GroupCount; i++)
    {
      const SID_AND_ATTRIBUTES & grp = groups->Groups[i];
      char * strsid;
      if (!ConvertSidToStringSid (grp.Sid, &strsid))
	return winerror ("ConvertSidToStringSid");

      // Any match with group options ?
      bool disable_sid = false;
      for (int j = 0; j < num_group_args && !disable_sid; j++)
	if (match_strsid (strsid, group_args[j]))
	  disable_sid = true;
      if (!g_flag)
	disable_sid = !disable_sid;

      bool restrict_sid = false;
      for (int j = 0; j < num_restr_args && !restrict_sid; j++)
	if (match_strsid (strsid, restr_args[j]))
	  restrict_sid = restr_used[j] = true;

      // Ignore special SIDs.
      bool ignore_sid = false;
      if ((grp.Attributes
	   & (SE_GROUP_USE_FOR_DENY_ONLY|SE_GROUP_LOGON_ID))
	  && (disable_sid || restrict_sid))
	{
	  disable_sid = restrict_sid = false;
	  ignore_sid = true;
	}

      if (verbose && (verbose > 1 || (disable_sid || restrict_sid)))
	{
	  // Print group info
	  printf ("%c%c  %s%s%s%s%s",
	    (ignore_sid ? 'i' : disable_sid ? 'd' : ' '),
	    (restrict_sid ? 'r' : ' '), strsid,
	    (grp.Attributes & SE_GROUP_ENABLED
	     ? " [enabled]" : ""),
	    (grp.Attributes & SE_GROUP_ENABLED_BY_DEFAULT
	     ? " [default]" : ""),
	    (grp.Attributes & SE_GROUP_USE_FOR_DENY_ONLY
	     ? " [deny_only]" : ""),
	    (grp.Attributes & SE_GROUP_LOGON_ID
	     ? " [logon_id]"  : ""));
	  const struct group * g = strsid_to_group (strsid);
	  if (g)
	    printf (" gid=%lu(%s)", g->gr_gid, g->gr_name);
	  printf ("\n");
	}

      LocalFree(strsid);

      if (disable_sid)
	{
	  sids_to_disable[num_sids_to_disable].Sid = grp.Sid;
	  sids_to_disable[num_sids_to_disable].Attributes = 0;
	  num_sids_to_disable++;
        }
      if (restrict_sid)
	{
          sids_to_restrict[num_sids_to_restrict].Sid = grp.Sid;
	  sids_to_restrict[num_sids_to_restrict].Attributes = 0;
	  num_sids_to_restrict++;
        }
    }

  // Add restricted SIDs not matched above.
  for (int i = 0; i < num_restr_args; i++)
  {
    if (restr_used[i])
      continue;
    const char * strsid = restr_args[i];
    if (!is_strsid (strsid))
      continue;
    PSID sid; // TODO: Never freed.
    if (!ConvertStringSidToSid ((char *) strsid, &sid))
	return winerror ("ConvertStringSidToSid");

    if (verbose)
      {
	printf ("r   %s", strsid);
	const struct group * g = strsid_to_group (strsid);
	if (g)
	  printf (" gid=%lu(%s)", g->gr_gid, g->gr_name);
	printf ("\n");
      }

    sids_to_restrict[num_sids_to_restrict].Sid = sid;
    sids_to_restrict[num_sids_to_restrict].Attributes = 0;
    num_sids_to_restrict++;
  }

  // Get privileges.
  char privs_buf[sizeof(DWORD) + max_privs * sizeof(LUID_AND_ATTRIBUTES)];
  TOKEN_PRIVILEGES * privs = (TOKEN_PRIVILEGES *)privs_buf;
  if (!GetTokenInformation (proc_token, TokenPrivileges, privs, sizeof(privs_buf), &size))
    return winerror ("GetTokenInformation");

  // Collect LUIDs of privileges to disable.
  LUID_AND_ATTRIBUTES privs_to_delete[max_privs];
  int num_privs_to_delete = 0;

  for (DWORD i = 0; i < privs->PrivilegeCount; i++)
    {
      LUID_AND_ATTRIBUTES & priv = privs->Privileges[i];
      char name[100];
      size = sizeof(name);
      if (!LookupPrivilegeName (NULL, &priv.Luid, name, &size))
	return winerror ("LookupPrivilegeName");

      // Any match with privilege options ?
      bool delete_priv = false;
      for (int j = 0; j < num_priv_args && !delete_priv; j++)
	if (match_priv (name, priv_args[j]))
	  delete_priv = true;
      if (!p_flag)
	delete_priv = !delete_priv;

      if (verbose && (verbose > 1 || delete_priv))
	printf("%c   %s%s%s\n", (delete_priv ? 'd' : ' '), name,
	       (priv.Attributes & SE_PRIVILEGE_ENABLED
	        ? " [enabled]" : ""),
	       (priv.Attributes & SE_PRIVILEGE_ENABLED_BY_DEFAULT
		? " [default]" : ""));

      if (delete_priv)
	{
	  privs_to_delete[num_privs_to_delete].Luid = priv.Luid;
	  privs_to_delete[num_privs_to_delete].Attributes = 0;
	  num_privs_to_delete++;
	}
    }

  // Create restricted token.
  HANDLE restr_token;
  if (!CreateRestrictedToken (proc_token, 0,
                              num_sids_to_disable,  sids_to_disable,
                              num_privs_to_delete,  privs_to_delete,
                              num_sids_to_restrict, sids_to_restrict,
                              &restr_token))
    return winerror("CreateRestrictedToken");

  CloseHandle (proc_token);

  // Change to restricted token.
  if (cygwin_internal (CW_SET_EXTERNAL_TOKEN, restr_token, CW_TOKEN_RESTRICTED))
    return cygerror ("cygwin_internal (CW_SET_EXTERNAL_TOKEN, ...)");

  if (setuid (geteuid ()))
    return cygerror ("setuid");

  // exec command.
  if (verbose)
    {
      printf ("\nexec ");
      for (int i = optind; i < argc-1; i++)
	printf ("'%s' ", argv[i]);
      printf ("'%s'\n", argv[argc-1]);
    }
  fflush(stdout);

  execvp (argv[optind], argv + optind);

  return cygerror (argv[optind]);
}

static const char *
getVersion ()
{
  return versionID;
}

static void
printTopDescription (FILE * f, const char *name)
{
  fprintf (f, "%s is part of cygutils version %s\n", name, getVersion ());
  fprintf (f, "%s was originally authored by %s\n", name, AUTHORS);
  fprintf (f, "  Execute COMMAND with a restricted access token\n\n");
}

static void
printBottomDescription (FILE * f, const char *name)
{
  fprintf(f,
    "If no group or privilege option is specified, '-l -d -m' is the default.\n"
    "Options with GROUP and PRIV parameter may be specified more than once.\n"
    "GROUP may be specified as a SID, a regular expression matching SIDs\n"
    "(must start with 'S-'), a numeric group id, or a group name.\n"
    "PRIV name match is not case sensitive, prefix 'Se' and suffix 'Privilege'\n"
    "may be omitted.\n\n");
}

static void
printLicense (FILE * f, const char *name)
{
  fprintf (f,
           "This program is free software: you can redistribute it and/or modify\n"
           "it under the terms of the GNU General Public License as published by\n"
           "the Free Software Foundation, either version 3 of the License, or\n"
           "(at your option) any later version.\n\n"
           "This program is distributed in the hope that it will be useful,\n"
           "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
           "GNU General Public License for more details.\n\n"
           "You should have received a copy of the GNU General Public License\n"
           "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n"
           "See the COPYING file for full license information.\n");
}

static void
help (FILE * f, const char *name)
{
  printTopDescription (f, name);
  usageCore (f, name);
  printBottomDescription (f, name);
}

static void
version (FILE * f, const char *name)
{
  printTopDescription (f, name);
}

static void
license (FILE * f, const char *name)
{
  printTopDescription (f, name);
  printLicense (f, name);
}
