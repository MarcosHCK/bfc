/* Copyright 2021-2025 MarcosHCK
 * This file is part of bfc (BrainFuck Compiler).
 *
 * bfc (BrainFuck Compiler) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bfc (BrainFuck Compiler) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bfc (BrainFuck Compiler).  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <config.h>
#include <collect.h>

#define BFC_COLLECT_ERROR (bfc_collect_error_quark ())
#define BFC_COLLECT_ERROR_FAILED (0)
G_DEFINE_QUARK (bfc-collect-error-quark, bfc_collect_error);
G_STATIC_ASSERT (LLVMCodeModelLarge == MMODEL_LARGE);
G_STATIC_ASSERT (LLVMRelocDynamicNoPic == RELOC_DYNNOPIC);

#define THROW(...) \
  G_STMT_START { \
    g_set_error \
    (error, \
     BFC_COLLECT_ERROR, \
     BFC_COLLECT_ERROR_FAILED, \
      __VA_ARGS__); \
    return; \
  } G_STMT_END
#define DELEGATE(func,...) \
  G_STMT_START { \
    GError* __tmperr = NULL; \
    (func) (__VA_ARGS__, &__tmperr); \
    if (G_UNLIKELY (__tmperr != NULL)) \
    { \
      g_propagate_error (error, __tmperr); \
      return; \
    } \
  } G_STMT_END

static inline void
checkpc (guint level, guint mmodel, GError** error)
{
  guint needed = 0;

  needed |= ((level & 1) ? MMODEL_SMALL : 0);
  needed |= ((level & 2) ? MMODEL_LARGE : 0);

  if (mmodel == MMODEL_DEFAULT)
    mmodel = needed;
  else
  if (((needed == MMODEL_SMALL) && !(MMODEL_SMALL >= mmodel && mmodel >= MMODEL_TINY))
    || ((needed == MMODEL_LARGE) && !(MMODEL_LARGE >= mmodel && mmodel >= MMODEL_KERNEL)))
    THROW ("Address model %s incompatible with pic settings", mmodel);
}

void
collect_codegen (BfcOptions* opt, gboolean static_, guint pic, guint pie, const gchar* mmodel, GError** error)
{
  guint mmodel_ = 0;
  guint needed = 0;
  guint level = 0;
  guint reloc = 0;

  if (mmodel == NULL)
  {
    mmodel_ = MMODEL_DEFAULT;
    mmodel = "default";
  }
  else
  if (!g_strcmp0 (mmodel, "default"))
    mmodel_ = MMODEL_DEFAULT;
  else
  if (!g_strcmp0 (mmodel, "tiny"))
    mmodel_ = MMODEL_TINY;
  else
  if (!g_strcmp0 (mmodel, "small"))
    mmodel_ = MMODEL_SMALL;
  else
  if (!g_strcmp0 (mmodel, "kernel"))
    mmodel_ = MMODEL_KERNEL;
  else
  if (!g_strcmp0 (mmodel, "medium"))
    mmodel_ = MMODEL_MEDIUM;
  else
  if (!g_strcmp0 (mmodel, "large"))
    mmodel_ = MMODEL_LARGE;
  else
  {
    g_set_error
    (error,
     BFC_COLLECT_ERROR,
     BFC_COLLECT_ERROR_FAILED,
     "Unknown address mode %s",
      mmodel);
    return;
  }

  if (pie == 3)
    THROW ("Can not have both pie and PIE", mmodel);
  if (pic == 3)
    THROW ("Can not have both pic and PIC");
  if ((pie | pic) == 3)
    THROW ("Can not have pic and PIE or like");

  DELEGATE (checkpc, pic, mmodel_);
  DELEGATE (checkpc, pie, mmodel_);

  reloc |= ((static_) ? RELOC_STATIC : 0);
  reloc |= ((pie > 0 || pic > 0) ? RELOC_PIC : 0);

  opt->mmodel = mmodel_;
  opt->static_ = static_;
  opt->reloc = reloc;
  opt->pic = pic;
  opt->pie = pie;
}

void
collect_machine (BfcOptions* opt, const gchar* arch, const gchar* tune, const gchar* features, GError** error)
{
  LLVMTargetMachineRef machine = NULL;
  LLVMTargetRef target = NULL;

  if (arch != NULL)
  {
    LLVMTargetRef iter;
    LLVMInitializeAllTargets ();
    LLVMInitializeAllTargetInfos ();

    iter = LLVMGetFirstTarget ();
    while (iter != NULL)
    {
      if (!g_strcmp0 (arch, LLVMGetTargetName (iter)))
      {
        target = iter;
        break;
      }

      iter = LLVMGetNextTarget (iter);
    }

    if (target == NULL)
    {
      g_error ("Can't load architecture %s", arch);
    }
  }
  else
  {
    LLVMInitializeNativeTarget ();
    gchar* message = NULL;

    if (LLVMGetTargetFromTriple (LLVM_HOST_TRIPLE, &target, &message))
      g_error ("Can't load native triplet %s", message);
    arch = LLVM_HOST_TRIPLE;
  }

  machine = LLVMCreateTargetMachine (target, arch, tune, features, LLVMCodeGenLevelDefault, opt->reloc, opt->mmodel);

  if (machine == NULL)
  {
    g_set_error
    (error,
     BFC_COLLECT_ERROR,
     BFC_COLLECT_ERROR_FAILED,
     "Can't recreate target machine");
  }
  else
  {
    opt->arch = arch;
    opt->features = features;
    opt->machine = machine;
    opt->tune = tune;
  }
}
