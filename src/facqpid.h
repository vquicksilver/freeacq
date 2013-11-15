/*
 * freeacq is the legal property of Víctor Enríquez Miguel. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
#ifndef _FREEACQ_PID_H_
#define _FREEACQ_PID_H_

G_BEGIN_DECLS

#define FACQ_TYPE_PID (facq_pid_get_type ())
#define FACQ_PID(inst) (G_TYPE_CHECK_INSTANCE_CAST ((inst),FACQ_TYPE_PID, FacqPID))
#define FACQ_PID_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass),FACQ_TYPE_PID, FacqPIDClass))
#define FACQ_IS_PID(inst) (G_TYPE_CHECK_INSTANCE_TYPE ((inst),FACQ_TYPE_PID))
#define FACQ_IS_PID_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),FACQ_TYPE_PID))
#define FACQ_PID_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_CLASS ((inst),FACQ_TYPE_PID,FacqPIDClass))

typedef struct _FacqPID FacqPID;
typedef struct _FacqPIDClass FacqPIDClass;
typedef struct _FacqPIDPrivate FacqPIDPrivate;

struct _FacqPID {
	/*< private >*/
        GObject parent_instance;
        FacqPIDPrivate *priv;
};

struct _FacqPIDClass {
	/*< private >*/
	GObjectClass parent_class;
};

GType facq_pid_get_type(void) G_GNUC_CONST;

FacqPID *facq_pid_new(gdouble period,gdouble propo,gdouble integ,gdouble deriv,gdouble target);
void facq_pid_set_propo(FacqPID *pid,gdouble propo);
void facq_pid_set_integ(FacqPID *pid,gdouble integ);
void facq_pid_set_deriv(FacqPID *pid,gdouble deriv);
void facq_pid_set_target(FacqPID *pid,gdouble target);
gdouble facq_pid_compute(FacqPID *pid,gdouble input);
void facq_pid_free(FacqPID *pid);

G_END_DECLS

#endif
