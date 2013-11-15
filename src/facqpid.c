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
#include <glib.h>
#include <gio/gio.h>
#include "facqpid.h"

/**
 * SECTION:facqpid
 * @title:FacqPID
 * @include:facqpid.h
 * @short_description: A PID (Proportional-Integral-Derivative) controller
 * implementation.
 *
 * This module provides a PID (Proportional-Integral-Derivative) controller.
 * 
 * For using it you have to create a #FacqPID object first with the
 * facq_pid_new() function. To obtain the value of the control signal in the
 * k-esim iteration call facq_pid_compute(). When the #FacqPID is no longer
 * needed destroy it with facq_pid_free().
 *
 * <sect1 id="pid-tunning">
 *  <title>PID tunning</title>
 *  <para>
 *  Before the PID can be used, you must provide values for the proportional,
 *  integral and derivative terms (Three constant values). Various procedures
 *  for determining this values does exist, you can check the following links
 *  for various standard methods used by the control industry:
 *  </para>
 *  <itemizedlist>
 *   <listitem>
 *    <para> 
 *    <ulink url="http://ctms.engin.umich.edu/CTMS/index.php?example=Introduction&section=ControlPID">Introduction to PID controller design</ulink>
 *    </para>
 *   </listitem>
 *  <listitem>
 *    <para>
 *    <ulink url="http://www.ni.com/white-paper/3782/en">PID Theory explained</ulink>
 *    </para>
 *  </listitem>
 *  </itemizedlist>
 * </sect1>
 */

/**
 * FacqPIDClass:
 *
 * Class for all the #FacqPID objects.
 */

/**
 * FacqPID:
 *
 * Contains all the private values required by the PID operation.
 */

G_DEFINE_TYPE(FacqPID,facq_pid,G_TYPE_OBJECT);

enum {
	PROP_0,
	PROP_PERIOD,
	PROP_PROPO,
	PROP_INTEG,
	PROP_DERIV,
	PROP_TARGET
};

struct _FacqPIDPrivate {
	/*< private >*/
	gdouble period;
	gdouble propo;
	gdouble integ;
	gdouble deriv;
	gdouble target;
	gdouble err;
	gdouble err_sum;
	gdouble err_1;
};

static void facq_pid_get_property(GObject *self,guint property_id,GValue *value,GParamSpec *pspec)
{
	FacqPID *pid = FACQ_PID(self);

	switch(property_id){
	case PROP_PERIOD: g_value_set_double(value,pid->priv->period);
	break;
	case PROP_PROPO: g_value_set_double(value,pid->priv->propo);
	break;
	case PROP_INTEG: g_value_set_double(value,pid->priv->integ);
	break;
	case PROP_DERIV: g_value_set_double(value,pid->priv->deriv);
	break;
	case PROP_TARGET: g_value_set_double(value,pid->priv->target);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(pid,property_id,pspec);
	}
}

static void facq_pid_set_property(GObject *self,guint property_id,const GValue *value,GParamSpec *pspec)
{
	FacqPID *pid = FACQ_PID(self);

	switch(property_id){
	case PROP_PERIOD: pid->priv->period = g_value_get_double(value);
	break;
	case PROP_PROPO: pid->priv->propo = g_value_get_double(value);
	break;
	case PROP_INTEG: pid->priv->integ = g_value_get_double(value);
	break;
	case PROP_DERIV: pid->priv->deriv = g_value_get_double(value);
	break;
	case PROP_TARGET: pid->priv->target = g_value_get_double(value);
	break;
	default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID(pid,property_id,pspec);
	}
}

static void facq_pid_class_init(FacqPIDClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_type_class_add_private(klass,sizeof(FacqPIDPrivate));

	object_class->set_property = facq_pid_set_property;
	object_class->get_property = facq_pid_get_property;

	g_object_class_install_property(object_class,PROP_PROPO,
					g_param_spec_double("propo",
							    "Proportional",
							    "The proportional constant",
							    0,
							    G_MAXDOUBLE,
							    1,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT |
							    G_PARAM_STATIC_STRINGS ));

	g_object_class_install_property(object_class,PROP_INTEG,
					g_param_spec_double("integ",
							    "Integral",
							    "The integral term",
							    0,
							    G_MAXDOUBLE,
							    1,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT |
							    G_PARAM_STATIC_STRINGS ));

	g_object_class_install_property(object_class,PROP_DERIV,
					g_param_spec_double("deriv",
							    "Derivative",
							    "The derivative term",
							    0,
							    G_MAXDOUBLE,
							    1,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT |
							    G_PARAM_STATIC_STRINGS ));

	g_object_class_install_property(object_class,PROP_TARGET,
					g_param_spec_double("target",
							    "Target",
							    "The target value for the input signal",
							    0,
							    G_MAXDOUBLE,
							    1,
							    G_PARAM_READWRITE |
							    G_PARAM_CONSTRUCT |
							    G_PARAM_STATIC_STRINGS ));

	g_object_class_install_property(object_class,PROP_PERIOD,
					g_param_spec_double("period",
							  "The period",
							  "The sampling period of the input samples in seconds",
							  1e-9,
							  G_MAXDOUBLE,
							  1,
							  G_PARAM_READWRITE |
							  G_PARAM_CONSTRUCT_ONLY |
							  G_PARAM_STATIC_STRINGS ));
}

static void facq_pid_init(FacqPID *pid)
{
	pid->priv = G_TYPE_INSTANCE_GET_PRIVATE(pid,FACQ_TYPE_PID,FacqPIDPrivate);
	pid->priv->propo = 0;
	pid->priv->integ = 0;
	pid->priv->deriv = 0;
	pid->priv->target = 0;
	pid->priv->period = 0;
	pid->priv->err = 0;
	pid->priv->err_sum = 0;
	pid->priv->err_1 = 0;
}

/**
 * facq_pid_new:
 * @period: The sampling period of the input signal(In nanoseconds).
 * @propo: The proportional constant.
 * @integ: The integral constant.
 * @deriv: The derivative constant.
 * @target: The target value for the input signal.
 *
 * Creates a new #FacqPID object with the parameters given by the user.
 *
 * Returns: A new #FacqPID object.
 */
FacqPID *facq_pid_new(gdouble period,gdouble propo,gdouble integ,gdouble deriv,gdouble target)
{
	return g_object_new(FACQ_TYPE_PID,
			"period",period,
			"propo",propo,
			"integ",integ,
			"deriv",deriv,
			"target",target,
			NULL);
}

/**
 * facq_pid_set_propo:
 * @pid: A #FacqPID object.
 * @propo: The new value for the proportional constant.
 *
 * Changes the value of the proportional constant.
 */
void facq_pid_set_propo(FacqPID *pid,gdouble propo)
{
	g_return_if_fail(FACQ_IS_PID(pid));

	pid->priv->propo = propo;
}

/**
 * facq_pid_set_integ:
 * @pid: A #FacqPID object.
 * @integ: The new value for the integral constant.
 *
 * Changes the value of the integral constant.
 */
void facq_pid_set_integ(FacqPID *pid,gdouble integ)
{
	g_return_if_fail(FACQ_IS_PID(pid));

	pid->priv->integ = integ;
}

/**
 * facq_pid_set_deriv:
 * @pid: A #FacqPID object.
 * @deriv: The new value for the derivative constant.
 *
 * Changes the value of the derivative constant.
 */
void facq_pid_set_deriv(FacqPID *pid,gdouble deriv)
{
	g_return_if_fail(FACQ_IS_PID(pid));

	pid->priv->deriv = deriv;
}

/**
 * facq_pid_set_target:
 * @pid: A #FacqPID object.
 * @target: The new target value for the input signal.
 *
 * Changes the value of the target value for the input signal.
 */
void facq_pid_set_target(FacqPID *pid,gdouble target)
{
	g_return_if_fail(FACQ_IS_PID(pid));

	pid->priv->target = target;
}

/**
 * facq_pid_compute:
 * @pid: A #FacqPID object.
 * @input: The k-esim sample of the input signal.
 *
 * Computes the value for the k-esim sample of the control signal, using the
 * k-esim sample of the input signal.
 *
 * Returns: The k-esim sample of the control signal, c(k).
 */
gdouble facq_pid_compute(FacqPID *pid,gdouble input)
{
	gdouble control = 0, err = 0, integ = 0, deriv = 0;

	g_return_val_if_fail(FACQ_IS_PID(pid),0);

	/* c(k) = K[ 
	 *           e(k)                      +  
	 *	     T/Ti * sum(e(0)+...+e(k)) +
	 *	     Td/T * (e(k)-e(k-1)
	 *	   ]
	 */

	err = pid->priv->target-input;
	pid->priv->err_sum += err;
	
	if(pid->priv->integ != 0)
		integ = (pid->priv->period/pid->priv->integ) * pid->priv->err_sum;
	
	if(pid->priv->deriv != 0)
		deriv = (pid->priv->deriv/pid->priv->period) * (err - pid->priv->err_1);

	control = pid->priv->propo * ( err + integ + deriv );

	pid->priv->err_1 = err;
	
	return control;
}

/**
 * facq_pid_free:
 * @pid: A #FacqPID object.
 *
 * Destroys the #FacqPID, @pid, object.
 */
void facq_pid_free(FacqPID *pid)
{
	g_return_if_fail(FACQ_IS_PID(pid));
	g_object_unref(G_OBJECT(pid));
}
